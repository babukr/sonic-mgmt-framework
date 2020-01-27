/*
###########################################################################
#
# Copyright 2019 Dell, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
###########################################################################
*/

#include "private.h"
#include "nos_extn.h"

#include <pthread.h>
#include <unistd.h>
#include <syslog.h>
#include <stdint.h>

pthread_mutex_t lock;

void *rest_token_refresh(void *vargp){
    int expiry  = (intptr_t)vargp;
    int interval;
    while(1) {
        interval = 0.8 * expiry; 
        syslog(LOG_DEBUG, "Sleeping for %d of %d", interval, expiry);

        /* Sleep for 80% of the interval */
        sleep(interval);

        pthread_mutex_lock(&lock);

        rest_token_fetch(&expiry);

        pthread_mutex_unlock(&lock);
    }
}

int clish_restcl_init() {
    pthread_t thread_id;

    pthread_mutex_init(&lock, NULL);

    int expiry = 30;
    rest_token_fetch(&expiry);
    syslog(LOG_DEBUG, "Token expires in %d seconds", expiry);
    
    pthread_create(&thread_id, NULL, rest_token_refresh, (void*)(long)expiry);
    return 0;
}

CLISH_PLUGIN_SYM(clish_restcl)
{
    char *cmd = clish_shell__get_full_line(clish_context);
    
    pthread_mutex_lock(&lock);

    int ret = rest_cl(cmd, script);

    pthread_mutex_unlock(&lock);

    return ret;
}

CLISH_PLUGIN_SYM(clish_pyobj)
{
    char *cmd = clish_shell__get_full_line(clish_context);

    pthread_mutex_lock(&lock);
    int ret = call_pyobj(cmd, script);
    pthread_mutex_unlock(&lock);

    return ret;
}

void nos_extn_init() {
    rest_update_user_info();
    pyobj_init();
    clish_restcl_init();
}

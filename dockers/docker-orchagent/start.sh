#!/usr/bin/env bash

mkdir -p /etc/swss/config.d/

sonic-cfggen -d -y /etc/sonic/sonic_version.yml -t /usr/share/sonic/templates/switch.json.j2 > /etc/swss/config.d/switch.json
sonic-cfggen -d -t /usr/share/sonic/templates/ipinip.json.j2 > /etc/swss/config.d/ipinip.json
sonic-cfggen -d -t /usr/share/sonic/templates/ports.json.j2 > /etc/swss/config.d/ports.json

# Executed HWSKU specific initialization tasks.
if [ -x /usr/share/sonic/hwsku/hwsku-init ]; then
    /usr/share/sonic/hwsku/hwsku-init
fi

export platform=`sonic-cfggen -H -v DEVICE_METADATA.localhost.platform`

rm -f /var/run/rsyslogd.pid

supervisorctl start rsyslogd

supervisorctl start orchagent

supervisorctl start restore_neighbors

supervisorctl start portsyncd

supervisorctl start gearsyncd

supervisorctl start neighsyncd

supervisorctl start fdbsyncd

supervisorctl start swssconfig

supervisorctl start vrfmgrd

supervisorctl start vlanmgrd

supervisorctl start intfmgrd

supervisorctl start portmgrd

supervisorctl start buffermgrd

supervisorctl start enable_counters

supervisorctl start nbrmgrd

supervisorctl start vxlanmgrd

supervisorctl start aclsvcd

# Set any scaling constants in Linux
if [ -f /usr/bin/update_proc_variables ]; then
    /usr/bin/update_proc_variables
fi
#Start the arp_update by default
# This is need to resolve arp for static neighbour entries configured for static route
supervisorctl start arp_update

# Start arp_update when VLAN exists
#VLAN=`sonic-cfggen -d -v 'VLAN.keys() | join(" ") if VLAN'`
#if [ "$VLAN" != "" ]; then
#    supervisorctl start arp_update
#fi

# Delay so that aclsvcd reconciliation completes
# before ip helper acl rules are applied after warmboot
# Iphelper rules are dynamically added and get lost
#  in reconciliation by aclmgr if iphelper starts prior
sleep 5
supervisorctl start iphelpermgrd

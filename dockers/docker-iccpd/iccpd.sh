#!/usr/bin/env bash

function start_app {
    rm -f /var/run/iccpd/*
    stpiccpsyncd &
    mclagsyncd &
    iccpd
}

function clean_up {
    pkill -9 stpiccpsyncd
    pkill -9 mclagsyncd
    pkill -9 iccpd
    exit
}

trap clean_up SIGTERM SIGKILL
start_app
read

#!/usr/bin/env python
# Script to stop and start the respective platforms default services. 
# This will be used while switching the pddf->non-pddf mode and vice versa
import os
import sys
import commands

def check_pddf_support():
    return True

def stop_platform_svc():
    status, output = commands.getstatusoutput("systemctl stop as7712-platform-init.service")
    if status:
        print "Stop as7712-platform-init.service failed %d"%status
        return False
    status, output = commands.getstatusoutput("systemctl disable as7712-platform-init.service")
    if status:
        print "Disable as7712-platform-init.service failed %d"%status
        return False

    # HACK , stop the pddf-platform-init service if it is active
    status, output = commands.getstatusoutput("systemctl stop pddf-platform-init.service")
    if status:
        print "Stop pddf-platform-init.service along with other platform serives failed %d"%status
        return False

    return True
    
def start_platform_svc():
    status, output = commands.getstatusoutput("systemctl enable as7712-platform-init.service")
    if status:
        print "Enable as7712-platform-init.service failed %d"%status
        return False
    status, output = commands.getstatusoutput("systemctl start as7712-platform-init.service")
    if status:
        print "Start as7712-platform-init.service failed %d"%status
        return False

    return True

def start_platform_pddf():
    status, output = commands.getstatusoutput("systemctl start pddf-platform-init.service")
    if status:
        print "Start pddf-platform-init.service failed %d"%status
        return False
    
    return True

def stop_platform_pddf():
    status, output = commands.getstatusoutput("systemctl stop pddf-platform-init.service")
    if status:
        print "Stop pddf-platform-init.service failed %d"%status
        return False

    return True

def main():
    pass

if __name__ == "__main__":
    main()


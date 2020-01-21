#!/usr/bin/python
# On Z9264f, the BaseBoard Management Controller is an
# autonomous subsystem provides monitoring and management
# facility independent of the host CPU. IPMI standard
# protocol is used with ipmitool to fetch sensor details.
# Current script support X00 board only. X01 support will
# be added soon. This provies support for the
# following objects:
#   * Onboard temperature sensors
#   * FAN trays
#   * PSU


import os
import sys
import logging
import subprocess
import commands

Z9264F_MAX_FAN_TRAYS = 4
Z9264F_MAX_PSUS = 2
IPMI_SENSOR_DATA = "ipmitool sdr list"
IPMI_SENSOR_DUMP = "/tmp/sdr"

FAN_PRESENCE = "FAN{0}_prsnt"
PSU_PRESENCE = "PSU{0}_state"
# Use this for older firmware
# PSU_PRESENCE="PSU{0}_prsnt"
IPMI_PSU1_DATA_DOCKER = "ipmitool raw 0x04 0x2d 0x31 |  awk '{print substr($0,9,1)}'"
IPMI_PSU2_DATA_DOCKER = "ipmitool raw 0x04 0x2d 0x32 |  awk '{print substr($0,9,1)}'"
IPMI_RAW_STORAGE_READ = "ipmitool raw 0x0a 0x11 {0} 0 0 0xa0"
ipmi_sdr_list = ""

# Dump sensor registers


def ipmi_sensor_dump():

    status = 1
    global ipmi_sdr_list
    ipmi_cmd = IPMI_SENSOR_DATA
    status, ipmi_sdr_list = commands.getstatusoutput(ipmi_cmd)

    if status:
        logging.error('Failed to execute:' + ipmi_sdr_list)
        sys.exit(0)

# Fetch a BMC register


def fetch_raw_fru_for_dir(dev_id, offset):
    Airflow_Direction = [' B2F', ' F2B']
    ret_status, ipmi_cmd_ret = commands.getstatusoutput(IPMI_RAW_STORAGE_READ.format(dev_id))
    if ret_status:
        logging.error('Failed to execute ipmitool')
    return Airflow_Direction[int((ipmi_cmd_ret.splitlines()[offset/16]).split(' ')[(offset%16+1)])]


def get_psu_airflow(psu_id):
    return fetch_raw_fru_for_dir(psu_id+5, 0x30)


def get_fan_airflow(fan_id):
    return fetch_raw_fru_for_dir(fan_id, 0x46)
    return Airflow_Direction[dir]

def get_pmc_register(reg_name):

    output = ""
    for item in ipmi_sdr_list.split("\n"):
        if reg_name in item:
            output = item.strip()

    if not output:
        print('\nFailed to fetch: ' + reg_name + ' sensor ')
        sys.exit(0)

    output = output.split('|')[1]

    logging.basicConfig(level=logging.DEBUG)
    return output


# Print the information for temperature sensors


def print_temperature_sensors():

    print("\nOnboard Temperature Sensors:")

    print '  PT_Left_temp:                   ',\
        (get_pmc_register('PT_Left_temp'))
    print '  PT_Mid_temp:                    ',\
        (get_pmc_register('PT_Mid_temp'))
    print '  PT_Right_temp:                  ',\
        (get_pmc_register('PT_Right_temp'))
    print '  Broadcom Temp:                  ',\
        (get_pmc_register('TC_Near_temp'))
    print '  Inlet Airflow Temp:             ',\
        (get_pmc_register('ILET_AF_temp'))
    print '  CPU Temp:                       ',\
        (get_pmc_register('CPU_Near_temp'))
    print '  CPU Near Temp:                  ',\
        (get_pmc_register('CPU_temp'))
    print '  PSU FAN AirFlow Temperature 1:  ',\
        (get_pmc_register('PSU1AF_temp'))
    print '  PSU FAN AirFlow Temperature 2:  ',\
        (get_pmc_register('PSU2AF_temp'))

ipmi_sensor_dump()

print_temperature_sensors()


# Print the information for 1 Fan Tray

def print_fan_tray(tray):

    Fan_Status = [' Normal', ' Abnormal', ' no reading']

    print '  Fan Tray ' + str(tray) + ':'

    if (tray == 1):

        if(int(get_pmc_register('FAN1_prsnt'), 16)):
            fan1_status = int(get_pmc_register('Fan1_Front_state'), 16)
            fan2_status = int(get_pmc_register('Fan1_Rear_state'), 16)
            # BMC taking some time to update
            if(fan1_status > 2 or fan2_status > 2):
                fan1_status = 2
                fan2_status = 2
        else:
            fan1_status = 2
            fan2_status = 2

        print '    Fan1 Speed:                   ',\
            get_pmc_register('FAN1_Front_rpm')
        print '    Fan2 Speed:                   ',\
            get_pmc_register('FAN1_Rear_rpm')
        print '    Fan1 State:                   ',\
            Fan_Status[fan1_status]
        print '    Fan2 State:                   ',\
            Fan_Status[fan2_status]
        print '    AirFlow:                      ',\
            get_fan_airflow(tray)

    elif (tray == 2):

        if(int(get_pmc_register('FAN2_prsnt'), 16)):
            fan1_status = int(get_pmc_register('Fan2_Front_state'), 16)
            fan2_status = int(get_pmc_register('Fan2_Rear_state'), 16)
            # BMC taking some time to update
            if(fan1_status > 2 or fan2_status > 2):
                fan1_status = 2
                fan2_status = 2
        else:
            fan1_status = 2
            fan2_status = 2

        print '    Fan1 Speed:                   ',\
            get_pmc_register('FAN2_Front_rpm')
        print '    Fan2 Speed:                   ',\
            get_pmc_register('FAN2_Rear_rpm')
        print '    Fan1 State:                   ',\
            Fan_Status[fan1_status]
        print '    Fan2 State:                   ',\
            Fan_Status[fan2_status]
        print '    AirFlow:                      ',\
            get_fan_airflow(tray)

    elif (tray == 3):

        if(int(get_pmc_register('FAN3_prsnt'), 16)):
            fan1_status = int(get_pmc_register('Fan3_Front_state'), 16)
            fan2_status = int(get_pmc_register('Fan3_Rear_state'), 16)
            # BMC taking some time to update
            if(fan1_status > 2 or fan2_status > 2):
                fan1_status = 2
                fan2_status = 2
        else:
            fan1_status = 2
            fan2_status = 2

        print '    Fan1 Speed:                   ',\
            get_pmc_register('FAN3_Front_rpm')
        print '    Fan2 Speed:                   ',\
            get_pmc_register('FAN3_Rear_rpm')
        print '    Fan1 State:                   ',\
            Fan_Status[fan1_status]
        print '    Fan2 State:                   ',\
            Fan_Status[fan2_status]
        print '    AirFlow:                      ',\
            get_fan_airflow(tray)

    elif (tray == 4):

        if(int(get_pmc_register('FAN4_prsnt'), 16)):
            fan1_status = int(get_pmc_register('Fan4_Front_state'), 16)
            fan2_status = int(get_pmc_register('Fan4_Rear_state'), 16)
            # BMC taking some time to update
            if(fan1_status > 2 or fan2_status > 2):
                fan1_status = 2
                fan2_status = 2
        else:
            fan1_status = 2
            fan2_status = 2

        print '    Fan1 Speed:                   ',\
            get_pmc_register('FAN4_Front_rpm')
        print '    Fan2 Speed:                   ',\
            get_pmc_register('FAN4_Rear_rpm')
        print '    Fan1 State:                   ',\
            Fan_Status[fan1_status]
        print '    Fan2 State:                   ',\
            Fan_Status[fan2_status]
        print '    AirFlow:                      ',\
            get_fan_airflow(tray)


print('\nFan Trays:')

for tray in range(1, Z9264F_MAX_FAN_TRAYS + 1):
    fan_presence = FAN_PRESENCE.format(tray)
    if (get_pmc_register(fan_presence)):
        print_fan_tray(tray)
    else:
        print '\n  Fan Tray ' + str(tray + 1) + ':     Not present'

    def get_psu_presence(index):
        """
        Retrieves the presence status of power supply unit (PSU) defined
                by index <index>
        :param index: An integer, index of the PSU of which to query status
        :return: Boolean, True if PSU is plugged, False if not
        """
        status = 0
        ret_status = 1
	ipmi_cmd_ret = '0'

        if index == 1:
           ret_status, ipmi_cmd_ret = commands.getstatusoutput(IPMI_PSU1_DATA_DOCKER)
        elif index == 2:
           ret_status, ipmi_cmd_ret = commands.getstatusoutput(IPMI_PSU2_DATA_DOCKER)

        psu_status = ipmi_cmd_ret

        return (int(psu_status, 16)& 1)

    def get_psu_status(index):
        """
        Retrieves the presence status of power supply unit (PSU) defined
                by index <index>
        :param index: An integer, index of the PSU of which to query status
        :return: Boolean, True if PSU is plugged, False if not
        """
        status = 0
        ret_status = 1
        ipmi_cmd_ret = 'f'

        if index == 1:
           ret_status, ipmi_cmd_ret = commands.getstatusoutput(IPMI_PSU1_DATA_DOCKER)
        elif index == 2:
           ret_status, ipmi_cmd_ret = commands.getstatusoutput(IPMI_PSU2_DATA_DOCKER)

        if ret_status:
            logging.error('Failed to execute ipmitool')

        psu_status = ipmi_cmd_ret

        return (not int(psu_status, 16) > 1)


# Print the information for PSU1, PSU2
def print_psu(psu):

    # PSU FAN details
    if (psu == 1):

        print '    PSU1:'
        print '       FAN Temperature:              ',\
            get_pmc_register('PSU1_Normal_temp')
        print '       FAN RPM:                      ',\
            get_pmc_register('PSU1_rpm')
        print '       Input Voltage:                ',\
            get_pmc_register('PSU1_In_volt')
        print '       Output Voltage:               ',\
            get_pmc_register('PSU1_Out_volt')
        print '       Input Power:                  ',\
            get_pmc_register('PSU1_In_watt')
        print '       Output Power:                 ',\
            get_pmc_register('PSU1_Out_watt')
        print '       Input Current:                ',\
            get_pmc_register('PSU1_In_amp')
        print '       Output Current:               ',\
            get_pmc_register('PSU1_Out_amp')
        print '       AirFlow:                      ',\
            get_psu_airflow(psu)

    else:

        print '    PSU2:'
        print '       FAN Temperature:              ',\
            get_pmc_register('PSU1_Normal_temp')
        print '       FAN RPM:                      ',\
            get_pmc_register('PSU2_rpm')
        print '       Input Voltage:                ',\
            get_pmc_register('PSU2_In_volt')
        print '       Output Voltage:               ',\
            get_pmc_register('PSU2_Out_volt')
        print '       Input Power:                  ',\
            get_pmc_register('PSU2_In_watt')
        print '       Output Power:                 ',\
            get_pmc_register('PSU2_Out_watt')
        print '       Input Current:                ',\
            get_pmc_register('PSU2_In_amp')
        print '       Output Current:               ',\
            get_pmc_register('PSU2_Out_amp')
        print '       AirFlow:                      ',\
            get_psu_airflow(psu)


print('\nPSUs:')
for psu in range(1, Z9264F_MAX_PSUS + 1):
    if not get_psu_presence(psu):
        print '\n    PSU{}: Not present'.format(psu)
    elif not get_psu_status(psu):
	print '\n    PSU{}: Not OK'.format(psu)
    else:
        print_psu(psu)

print '    Total Power:                     ',\
    get_pmc_register('PSU_Total_watt')

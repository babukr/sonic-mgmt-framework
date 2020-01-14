#!/usr/bin/env python

#############################################################################
# Broadcom XLR/GTS 'eeprom' support
#
# Platform and model specific eeprom subclass, inherits from the base class,
# and provides the followings:
# - the eeprom format definition
# - specific encoder/decoder if there is special need
#
# Note: the file /usr/share/sonic/platform/sys_eeprom.bin is generated 
#       by the script brcm-xlr-gts-create-eeprom-file.py
#############################################################################

try:
    from sonic_eeprom import eeprom_tlvinfo
except ImportError, e:
    raise ImportError (str(e) + "- required module not found")


class board(eeprom_tlvinfo.TlvInfoDecoder):

    def __init__(self, name, path, cpld_root, ro):
        self.eeprom_path = "/usr/share/sonic/platform/sys_eeprom.bin"
        super(board, self).__init__(self.eeprom_path, 0, '', False, True)

    def serial_number_str(self, e):
        """Return service tag instead of serial number"""
        return "No service tag"

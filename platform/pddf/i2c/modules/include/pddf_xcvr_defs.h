/*
 * Copyright 2019 Broadcom. All rights reserved.
 * The term “Broadcom” refers to Broadcom Inc. and/or its subsidiaries.
 *
 * Description:
 *	Platform SFP defines/structures header file
 */

#ifndef __PDDF_XCVR_DEFS_H__
#define __PDDF_XCVR_DEFS_H__


#define MAX_NUM_XCVR 5
#define MAX_XCVR_ATTRS 20


typedef struct XCVR_ATTR
{
    char aname[32];                    // attr name, taken from enum xcvr_sysfs_attributes
	char devtype[32];       // either a 'eeprom' or 'cpld', or 'pmbus' attribute
    uint32_t devaddr;
    uint32_t offset;
    uint32_t mask;
    uint32_t cmpval;
    uint32_t len;

	int (*pre_access)(void *client, void *data);
	int (*do_access)(void *client, void *data);
	int (*post_access)(void *client, void *data);

}XCVR_ATTR;

/* XCVR CLIENT DATA - PLATFORM DATA FOR XCVR CLIENT */
typedef struct XCVR_DATA
{
    int idx;                    // xcvr index
	XCVR_ATTR xcvr_attr;
    int len;             // no of valid attributes for this xcvr client
    XCVR_ATTR xcvr_attrs[MAX_XCVR_ATTRS]; 
}XCVR_DATA;

typedef struct XCVR_PDATA
{
    int idx;                    // xcvr index
    unsigned short addr;      // i2c address of the device
    int len;             // no of valid attributes for this xcvr client
    XCVR_ATTR *xcvr_attrs; 
}XCVR_PDATA;


#define BIT_INDEX(i)            (1ULL << (i))

/* List of valid port types */
typedef enum xcvr_port_type_e {
    PDDF_PORT_TYPE_INVALID,
    PDDF_PORT_TYPE_NOT_PRESENT,
    PDDF_PORT_TYPE_SFP,
    PDDF_PORT_TYPE_SFP_PLUS,
    PDDF_PORT_TYPE_QSFP,
    PDDF_PORT_TYPE_QSFP_PLUS,
    PDDF_PORT_TYPE_QSFP28
} xcvr_port_type_t;

/*static int data_parsing_complete = 0;*/

/* Each client has this additional data
 */
struct xcvr_data {
    struct device       *xdev;
    struct mutex        update_lock;
    char                valid;           /* !=0 if registers are valid */
    unsigned long       last_updated;    /* In jiffies */
    int                 index;           /* port index */
    xcvr_port_type_t    port_type;
    uint64_t            present;
};

extern int board_i2c_cpld_read(unsigned short cpld_addr, u8 reg);
extern int board_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

#endif //__PDDF_XCVR_DEFS_H__

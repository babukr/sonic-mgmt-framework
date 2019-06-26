/*
 * Copyright 2019 Broadcom. All rights reserved.
 * The term “Broadcom” refers to Broadcom Inc. and/or its subsidiaries.
 *
 * Description:
 *	Platform PSU defines/structures header file
 */

#ifndef __PDDF_PSU_DEFS_H__
#define __PDDF_PSU_DEFS_H__


#define MAX_NUM_PSU 5
#define MAX_PSU_ATTRS 20
#define ATTR_NAME_LEN 32
#define STR_ATTR_SIZE 32
#define DEV_TYPE_LEN 32

/* Each client has this additional data 
 */

typedef struct PSU_DATA_ATTR
{
    char aname[ATTR_NAME_LEN];                    // attr name, taken from enum psu_sysfs_attributes
	char devtype[DEV_TYPE_LEN];       // either a 'eeprom' or 'cpld', or 'pmbus' attribute
    uint32_t devaddr;
    uint32_t offset;
    uint32_t mask;
    uint32_t cmpval;
    uint32_t len;
	void *access_data;

}PSU_DATA_ATTR;

typedef struct PSU_SYSFS_ATTR_DATA
{
	int index;
	unsigned short mode;
	ssize_t (*show)(struct device *dev, struct device_attribute *da, char *buf);
	ssize_t (*store)(struct device *dev, struct device_attribute *da, const char *buf, size_t count);
	int (*pre_access)(void *client, PSU_DATA_ATTR *adata, void *data);
	int (*do_access)(void *client, PSU_DATA_ATTR *adata, void *data);
	int (*post_access)(void *client, PSU_DATA_ATTR *adata, void *data);
	void *data;
} PSU_SYSFS_ATTR_DATA;

typedef struct PSU_SYSFS_ATTR_DATA_ENTRY
{
	char name[ATTR_NAME_LEN];
	PSU_SYSFS_ATTR_DATA *a_ptr;
} PSU_SYSFS_ATTR_DATA_ENTRY;


/* PSU CLIENT DATA - PLATFORM DATA FOR PSU CLIENT */
typedef struct PSU_DATA
{
    int idx;                    // psu index
	PSU_DATA_ATTR psu_attr;
    int len;             // no of valid attributes for this psu client
    PSU_DATA_ATTR psu_attrs[MAX_PSU_ATTRS]; 
}PSU_DATA;

typedef struct PSU_PDATA
{
    int idx;                    // psu index
    unsigned short addr;      // i2c address of the device
    int len;             // no of valid attributes for this psu client
    PSU_DATA_ATTR *psu_attrs; 
}PSU_PDATA;

extern int board_i2c_cpld_read(unsigned short cpld_addr, u8 reg);
extern int board_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value);

#endif //__PDDF_PSU_DEFS_H__

/*
 * Copyright 2019 Broadcom. All rights reserved.
 * The term “Broadcom” refers to Broadcom Inc. and/or its subsidiaries.
 *
 * Description
 *  PSU driver data structures
 */
#ifndef __PDDF_PSU_DRIVER_H__
#define __PDDF_PSU_DRIVER_H__

enum psu_sysfs_attributes {
    PSU_PRESENT,
    PSU_MODEL_NAME,
    PSU_POWER_GOOD,
	PSU_MFR_ID,
	PSU_SERIAL_NUM,
	PSU_FAN_DIR,
    PSU_V_OUT,
    PSU_I_OUT,
    PSU_P_OUT,
    PSU_FAN1_SPEED_RPM,
	PSU_ATTR_MAX
};


/* Every client has psu_data which is divided into per attribute data */
struct psu_attr_info {
	char				name[ATTR_NAME_LEN];
	struct mutex        update_lock;
    char                valid;           /* !=0 if registers are valid */
    unsigned long       last_updated;    /* In jiffies */
	u8					status;
	union {
		char strval[STR_ATTR_SIZE];
		int	 intval;
		u16	 shortval;
		u8   charval;
	}val;
};
struct psu_data {
	struct device			*hwmon_dev;
	u8						index;
	int						num_attr;
	struct attribute		*psu_attribute_list[MAX_PSU_ATTRS];
	struct attribute_group	psu_attribute_group;
	struct psu_attr_info	attr_info[MAX_PSU_ATTRS];
};


#endif //__PDDF_PSU_DRIVER_H__

/*
 * Copyright 2019 Broadcom. All rights reserved.
 * The term “Broadcom” refers to Broadcom Inc. and/or its subsidiaries.
 *
 * Description:
 *	Platform I2C client defines/structures header file
 */

#ifndef __PDDF_CLIENT_DEFS_H__
#define __PDDF_CLIENT_DEFS_H__


#include <linux/i2c.h>
#include <linux/platform_data/pca953x.h>
#include <linux/i2c/pca954x.h>


/*#define PDDF_DEBUG*/
#ifdef PDDF_DEBUG
#define pddf_dbg(...) printk(__VA_ARGS__)
#else
#define pddf_dbg(...) 
#endif


/*#define MAX_NUM_PSU 5*/
/*#define MAX_PSU_ATTRS 20*/
#define GEN_NAME_SIZE 32
#define ERR_STR_SIZE 128


typedef struct pddf_data_attribute{
        struct device_attribute dev_attr;
        int type;
        int len;
        char *addr;
		char *data;
}PDDF_ATTR;

#define PDDF_DATA_ATTR(_name, _mode, _show, _store, _type, _len, _addr, _data)    \
        struct pddf_data_attribute attr_##_name = { .dev_attr = __ATTR(_name, _mode, _show, _store),  \
          .type = _type , \
          .len = _len ,   \
          .addr = _addr,  \
		  .data = _data }


enum attribute_data_type {
    PDDF_CHAR,
    PDDF_UCHAR,
    PDDF_INT_HEX,    // integer represented in HEX
    PDDF_INT_DEC,    // integer represented in DECIMAL
    PDDF_USHORT,     // HEX
    PDDF_UINT32      // HEX
};





// PSU Specific details

typedef struct NEW_DEV_ATTR
{
    char i2c_type[GEN_NAME_SIZE];
    char i2c_name[GEN_NAME_SIZE];
    int parent_bus;
    char dev_type[GEN_NAME_SIZE];
    int dev_addr;
	char *data;
	int error;
	char errstr[ERR_STR_SIZE];

}NEW_DEV_ATTR;
extern NEW_DEV_ATTR pddf_data;

extern struct attribute_group pddf_clients_data_group;
extern  ssize_t store_pddf_data(struct device *dev, struct device_attribute *da, const char *buf, size_t count);
extern  ssize_t show_pddf_data(struct device *dev, struct device_attribute *da, char *buf);
struct kobject* get_device_i2c_kobj(void);
void set_attr_data(void * ptr);
void set_error_code(int, char *);
ssize_t show_error_code(struct device *dev, struct device_attribute *da, char *buf);
ssize_t show_all_devices(struct device *dev, struct device_attribute *da, char *buf);
void traverse_device_table(void );



/*Various Ops hook which can be used by vendors to provide some deviation from usual pddf functionality*/
struct pddf_ops_t 
{
	/*Module init ops*/
	int (*pre_init)(void);
	int (*post_init)(void);
	/*probe ops*/
	int (*pre_probe)(struct i2c_client *, const struct i2c_device_id *);
	int (*post_probe)(struct i2c_client *, const struct i2c_device_id *);
	/*remove ops*/
	int (*pre_remove)(struct i2c_client *);
	int (*post_remove)(struct i2c_client *);
	/*Module exit ops*/
	void (*pre_exit)(void);
    void (*post_exit)(void);
};


typedef struct PDEVICE
{
	struct hlist_node node;
	char name[GEN_NAME_SIZE];
	char *data;

}PDEVICE;

void add_device_table(char *name, void *ptr);


#endif //__PDDF_CLIENT_DEFS_H__

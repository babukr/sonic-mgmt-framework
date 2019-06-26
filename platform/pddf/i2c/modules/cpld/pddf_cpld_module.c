/*
 * Copyright 2019 Broadcom. All rights reserved. 
 * The term “Broadcom” refers to Broadcom Inc. and/or its subsidiaries.
 *
 * A pddf kernel module to create I2C client for a CPLD 
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/dmi.h>
#include <linux/kobject.h>
#include "pddf_client_defs.h"
#include "pddf_cpld_defs.h"


static ssize_t do_device_operation(struct device *dev, struct device_attribute *da, const char *buf, size_t count);
extern void *get_device_table(char *name);
extern void delete_device_table(char *name);


/* MUX CLIENT DATA */
PDDF_DATA_ATTR(dev_ops, S_IWUSR, NULL, do_device_operation, PDDF_CHAR, 8, NULL, (void*)&pddf_data);



static struct attribute *cpld_attributes[] = {
	&attr_dev_ops.dev_attr.attr,
	NULL
};

static const struct attribute_group pddf_cpld_client_data_group = {
	.attrs = cpld_attributes,
};


/*PDDF_DATA_ATTR(dev_ops, S_IWUSR, NULL, do_device_operation, PDDF_CHAR, 8, (void*)&pddf_attr, (void*)NULL);*/
static ssize_t do_device_operation(struct device *dev, struct device_attribute *da, const char *buf, size_t count)
{
	PDDF_ATTR *ptr = (PDDF_ATTR *)da;
	NEW_DEV_ATTR *device_ptr = (NEW_DEV_ATTR *)(ptr->data);
	struct i2c_adapter *adapter;
	static struct i2c_board_info board_info;
	struct i2c_client *client_ptr;

	/*pddf_dbg(KERN_ERR "%s: %s", __FUNCTION__, buf);*/
	/*pddf_dbg(KERN_ERR "Creating an I2C MUX client with parent_bus:0x%x, dev_type:%s, dev_addr:0x%x\n", device_ptr->parent_bus, device_ptr->dev_type, device_ptr->dev_addr);*/
	if (strncmp(buf, "add", strlen(buf)-1)==0)
	{
		adapter = i2c_get_adapter(device_ptr->parent_bus);
		
		if (strncmp(device_ptr->dev_type, "i2c_cpld", strlen("i2c_cpld"))==0)
		{
			board_info = (struct i2c_board_info) {
				.platform_data = (void *)NULL,
			};

			board_info.addr = device_ptr->dev_addr;
			strcpy(board_info.type, device_ptr->dev_type);

			/*pddf_dbg(KERN_ERR "Creating a client %s on 0x%x, platform_data 0x%x\n", board_info.type, board_info.addr, board_info.platform_data);*/
			client_ptr = i2c_new_device(adapter, &board_info);

			if (client_ptr != NULL) {
				i2c_put_adapter(adapter);
				pddf_dbg(KERN_ERR "Created %s client: 0x%x\n", device_ptr->i2c_name, client_ptr);
				add_device_table(device_ptr->i2c_name, (void*)client_ptr);
			}
			else {
				i2c_put_adapter(adapter);
				goto free_data;
			}

		}
		else
		{
			printk(KERN_ERR "%s: Unsupported type of cpld - unable to add i2c client\n", __FUNCTION__);
		}
	}
	else if (strncmp(buf, "delete", strlen(buf)-1)==0)
	{
		/*Get the i2c_client handle for the created client*/
		client_ptr = (struct i2c_client *)get_device_table(device_ptr->i2c_name);
		if (client_ptr)
		{
			pddf_dbg(KERN_ERR "Removing %s client: 0x%x\n", device_ptr->i2c_name, client_ptr);
			i2c_unregister_device(client_ptr);
			delete_device_table(device_ptr->i2c_name);
		}
		else
		{
			printk(KERN_ERR "Unable to get the client handle for %s\n", device_ptr->i2c_name);
		}
	}
	else
	{
		pddf_dbg(KERN_ERR "%s: Wrong value for dev_ops %s", __FUNCTION__, buf);
	}

free_data:
	/*TODO: free the device_ptr->data is dynamically allocated*/
	memset(device_ptr, 0 , sizeof(NEW_DEV_ATTR));

	return count;
}


static struct kobject *cpld_kobj;

int __init cpld_data_init(void)
{
	struct kobject *device_kobj;
	int ret = 0;


	pddf_dbg("CPLD_DATA MODULE.. init\n");

	device_kobj = get_device_i2c_kobj();
	if(!device_kobj) 
		return -ENOMEM;

	cpld_kobj = kobject_create_and_add("cpld", device_kobj);
	if(!cpld_kobj) 
		return -ENOMEM;
	
	
	ret = sysfs_create_group(cpld_kobj, &pddf_clients_data_group);
	if (ret)
    {
        kobject_put(cpld_kobj);
        return ret;
    }
	pddf_dbg("CREATED PDDF I2C CLIENTS CREATION SYSFS GROUP\n");

	ret = sysfs_create_group(cpld_kobj, &pddf_cpld_client_data_group);
	if (ret)
    {
		sysfs_remove_group(cpld_kobj, &pddf_clients_data_group);
        kobject_put(cpld_kobj);
        return ret;
    }
	pddf_dbg("CREATED PDDF I2C CLIENTS CREATION SYSFS GROUP\n");
	return ret;
}

void __exit cpld_data_exit(void)
{
	pddf_dbg("CPLD_DATA MODULE.. exit\n");
	sysfs_remove_group(cpld_kobj, &pddf_cpld_client_data_group);
	sysfs_remove_group(cpld_kobj, &pddf_clients_data_group);
    kobject_put(cpld_kobj);
    pddf_dbg(KERN_ERR "%s: Removed the kobjects for 'cpld'\n",__FUNCTION__);
    return;
}

module_init(cpld_data_init);
module_exit(cpld_data_exit);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("cpld platform data");
MODULE_LICENSE("GPL");

/*#endif*/

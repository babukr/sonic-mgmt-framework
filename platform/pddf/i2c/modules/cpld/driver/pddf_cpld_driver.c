/*
 * Copyright 2019 Broadcom. All rights reserved.
 * The term “Broadcom” refers to Broadcom Inc. and/or its subsidiaries.
 *
 * A pddf kernel driver module for CPLD
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/dmi.h>


//extern int hw_preaccess_func_cpld_mux_default(uint32_t unit, void *data);


static LIST_HEAD(cpld_client_list);
static struct mutex	 list_lock;

struct cpld_client_node {
	struct i2c_client *client;
	struct list_head   list;
};

/* Addresses scanned for board_i2c_cpld
 */
static const unsigned short normal_i2c[] = { 0x31, 0x32, 0x33, 0x35, 0x60, 0x61, 0x62, 0x64, I2C_CLIENT_END };

static void board_i2c_cpld_add_client(struct i2c_client *client)
{
	struct cpld_client_node *node = kzalloc(sizeof(struct cpld_client_node), GFP_KERNEL);
	
	if (!node) {
		dev_dbg(&client->dev, "Can't allocate cpld_client_node (0x%x)\n", client->addr);
		return;
	}
	
	node->client = client;
	
	mutex_lock(&list_lock);
	list_add(&node->list, &cpld_client_list);
	mutex_unlock(&list_lock);
}

static void board_i2c_cpld_remove_client(struct i2c_client *client)
{
	struct list_head		*list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int found = 0;
	
	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list)
	{
		cpld_node = list_entry(list_node, struct cpld_client_node, list);
		
		if (cpld_node->client == client) {
			found = 1;
			break;
		}
	}
	
	if (found) {
		list_del(list_node);
		kfree(cpld_node);
	}
	
	mutex_unlock(&list_lock);
}

static int board_i2c_cpld_probe(struct i2c_client *client,
			const struct i2c_device_id *dev_id)
{
	int status;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_dbg(&client->dev, "i2c_check_functionality failed (0x%x)\n", client->addr);
		status = -EIO;
		goto exit;
	}

	dev_dbg(&client->dev, "chip found\n");
	board_i2c_cpld_add_client(client);
	
	return 0;

exit:
	return status;
}

static int board_i2c_cpld_remove(struct i2c_client *client)
{
	board_i2c_cpld_remove_client(client);
	
	return 0;
}

static const struct i2c_device_id board_i2c_cpld_id[] = {
	{ "i2c_cpld", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, board_i2c_cpld_id);

static struct i2c_driver board_i2c_cpld_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name = "i2c_cpld",
	},
	.probe		= board_i2c_cpld_probe,
	.remove	   	= board_i2c_cpld_remove,
	.id_table	= board_i2c_cpld_id,
	.address_list = normal_i2c,
};

int board_i2c_cpld_read(unsigned short cpld_addr, u8 reg)
{
	struct list_head   *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int ret = -EPERM;
	
	//hw_preaccess_func_cpld_mux_default((uint32_t)cpld_addr, NULL);

	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list)
	{
		cpld_node = list_entry(list_node, struct cpld_client_node, list);
		
		if (cpld_node->client->addr == cpld_addr) {
			ret = i2c_smbus_read_byte_data(cpld_node->client, reg);
			break;
		}
	}
	
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(board_i2c_cpld_read);

int board_i2c_cpld_write(unsigned short cpld_addr, u8 reg, u8 value)
{
	struct list_head   *list_node = NULL;
	struct cpld_client_node *cpld_node = NULL;
	int ret = -EIO;
	
	//hw_preaccess_func_cpld_mux_default((uint32_t)cpld_addr, NULL);

	mutex_lock(&list_lock);

	list_for_each(list_node, &cpld_client_list)
	{
		cpld_node = list_entry(list_node, struct cpld_client_node, list);
		
		if (cpld_node->client->addr == cpld_addr) {
			ret = i2c_smbus_write_byte_data(cpld_node->client, reg, value);
			break;
		}
	}
	
	mutex_unlock(&list_lock);

	return ret;
}
EXPORT_SYMBOL(board_i2c_cpld_write);

static int __init board_i2c_cpld_init(void)
{
	mutex_init(&list_lock);
	return i2c_add_driver(&board_i2c_cpld_driver);
}

static void __exit board_i2c_cpld_exit(void)
{
	i2c_del_driver(&board_i2c_cpld_driver);
}
	
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("board_i2c_cpld driver");
MODULE_LICENSE("GPL");

module_init(board_i2c_cpld_init);
module_exit(board_i2c_cpld_exit);

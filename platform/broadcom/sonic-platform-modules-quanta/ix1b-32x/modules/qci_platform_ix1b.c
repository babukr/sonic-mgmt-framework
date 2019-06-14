/*
 *  Quanta IX1B platform driver
 *
 *
 *  Copyright (C) 2014 Quanta Computer inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/input.h>
#include <linux/input/sparse-keymap.h>
#include <linux/input-polldev.h>
#include <linux/rfkill.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/i2c/pca954x.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,12,0)
#include <linux/platform_data/pca953x.h>
#else
#include <linux/i2c/pca953x.h>
#endif
#include "qci_platform.h"

#define QUANTA_CPU_RGL

static struct pca954x_platform_mode pca9548sfp1_modes[] = {
	MUX_INFO(0x20, 1),
	MUX_INFO(0x21, 1),
	MUX_INFO(0x22, 1),
	MUX_INFO(0x23, 1),
	MUX_INFO(0x24, 1),
	MUX_INFO(0x25, 1),
	MUX_INFO(0x26, 1),
	MUX_INFO(0x27, 1),
};

static struct pca954x_platform_data pca9548sfp1_data = {
	.modes 		= pca9548sfp1_modes,
	.num_modes 	= 8,
};

static struct pca954x_platform_mode pca9548sfp2_modes[] = {
	MUX_INFO(0x28, 1),
	MUX_INFO(0x29, 1),
	MUX_INFO(0x2a, 1),
	MUX_INFO(0x2b, 1),
	MUX_INFO(0x2c, 1),
	MUX_INFO(0x2d, 1),
	MUX_INFO(0x2e, 1),
	MUX_INFO(0x2f, 1),
};

static struct pca954x_platform_data pca9548sfp2_data = {
	.modes 		= pca9548sfp2_modes,
	.num_modes 	= 8,
};

static struct pca954x_platform_mode pca9548sfp3_modes[] = {
	MUX_INFO(0x30, 1),
	MUX_INFO(0x31, 1),
	MUX_INFO(0x32, 1),
	MUX_INFO(0x33, 1),
	MUX_INFO(0x34, 1),
	MUX_INFO(0x35, 1),
	MUX_INFO(0x36, 1),
	MUX_INFO(0x37, 1),
};

static struct pca954x_platform_data pca9548sfp3_data = {
	.modes 		= pca9548sfp3_modes,
	.num_modes 	= 8,
};

static struct pca954x_platform_mode pca9548sfp4_modes[] = {
	MUX_INFO(0x38, 1),
	MUX_INFO(0x39, 1),
	MUX_INFO(0x3a, 1),
	MUX_INFO(0x3b, 1),
	MUX_INFO(0x3c, 1),
	MUX_INFO(0x3d, 1),
	MUX_INFO(0x3e, 1),
	MUX_INFO(0x3f, 1),
};

static struct pca954x_platform_data pca9548sfp4_data = {
	.modes 		= pca9548sfp4_modes,
	.num_modes 	= 8,
};

static struct pca954x_platform_mode pca9546_1_modes[] = {
	MUX_INFO(0x10, 1),
	MUX_INFO(0x11, 1),
	MUX_INFO(0x12, 1),
	MUX_INFO(0x13, 1),
};

static struct pca954x_platform_data pca9546_1_data = {
	.modes 		= pca9546_1_modes,
	.num_modes 	= 4,
};

static struct pca954x_platform_mode pca9546_2_modes[] = {
	MUX_INFO(0x14, 1),
	MUX_INFO(0x15, 1),
	MUX_INFO(0x16, 1),
	MUX_INFO(0x17, 1),
};

static struct pca954x_platform_data pca9546_2_data = {
	.modes 		= pca9546_2_modes,
	.num_modes 	= 4,
};

/* CPU gpio base defined in devicetree
 * default is ARCH_NR_GPIOS - 32, (256-32=224)
 * It must match devicetree default gpio base
 * */
#define PCA9555_PSU_GPIO_BASE	0x10
#define PCA9555_FAN_GPIO_BASE	0x20
#define PCA9555_ID_GPIO_BASE	0x30

#if defined(QUANTA_CPU_RGL)
#define PCA9555_CPU_LED_GPIO_BASE	0x80

static struct pca954x_platform_mode pca9546_cpu1_modes[] = {
	MUX_INFO(0x18, 1),
	MUX_INFO(0x19, 1),
	MUX_INFO(0x1a, 1),
	MUX_INFO(0x1b, 1),
};

static struct pca954x_platform_data pca9546_cpu1_data = {
	.modes 		= pca9546_cpu1_modes,
	.num_modes 	= 4,
};

static struct pca953x_platform_data pca9555_cpuled_data = {
	.gpio_base = PCA9555_CPU_LED_GPIO_BASE,
};
#endif

static struct pca953x_platform_data pca9555psu1_data = {
	.gpio_base = PCA9555_PSU_GPIO_BASE,
};

static struct pca953x_platform_data pca9555fan_data = {
	.gpio_base = PCA9555_FAN_GPIO_BASE,
};

static struct pca953x_platform_data pca9555ID_data = {
	.gpio_base = PCA9555_ID_GPIO_BASE,
};

static struct i2c_board_info ix1b_i2c_devices[] = {
	{
		I2C_BOARD_INFO("pca9546", 0x77),
		.platform_data = &pca9546_1_data,
	},
	{
		I2C_BOARD_INFO("pca9546", 0x72),
		.platform_data = &pca9546_2_data,
	},
	{
		I2C_BOARD_INFO("pca9555", 0x26),
		.platform_data = &pca9555psu1_data,
	},
	{
		I2C_BOARD_INFO("24c02", 0x54),
	},
	{
		I2C_BOARD_INFO("pca9548", 0x73),
		.platform_data = &pca9548sfp1_data,
	},
	{
		I2C_BOARD_INFO("pca9548", 0x74),
		.platform_data = &pca9548sfp2_data,
	},
	{
		I2C_BOARD_INFO("pca9548", 0x75),
		.platform_data = &pca9548sfp3_data,
	},
	{
		I2C_BOARD_INFO("pca9548", 0x76),
		.platform_data = &pca9548sfp4_data,
	},
	{
		I2C_BOARD_INFO("CPLD-QSFP28", 0x38),
	},
	{
		I2C_BOARD_INFO("CPLD-QSFP28", 0x39),
	},
	{
		I2C_BOARD_INFO("pca9555", 0x23),
		.platform_data = &pca9555ID_data,
	},
	{
		I2C_BOARD_INFO("pca9555", 0x25),
		.platform_data = &pca9555fan_data,
	},
	{
		I2C_BOARD_INFO("qci_pmbus_ix1b", 0x5f),
	},
	{
		I2C_BOARD_INFO("qci_pmbus_ix1b", 0x59),
	},
#if defined(QUANTA_CPU_RGL)
	{
		I2C_BOARD_INFO("pca9546", 0x71),
		.platform_data = &pca9546_cpu1_data,
	},
	{
		I2C_BOARD_INFO("pca9555", 0x20),
		.platform_data = &pca9555_cpuled_data,
	},
	{
		I2C_BOARD_INFO("optoe1", 0x50),
	},
#endif
};

static struct platform_driver ix1b_platform_driver = {
	.driver = {
		.name = "qci-ix1b",
		.owner = THIS_MODULE,
	},
};

static struct platform_led ix1b_leds[] = {
#if defined(QUANTA_CPU_RGL)
	LED_INFO(SYSLED_AMBER),
	LED_INFO(SYSLED_GREEN),
#endif
	LED_INFO(FRONT_PSU1_GREEN),
	LED_INFO(FRONT_PSU1_RED),
	LED_INFO(FRONT_PSU2_GREEN),
	LED_INFO(FRONT_PSU2_RED),
	LED_INFO(FRONT_FAN_GREEN),
	LED_INFO(FRONT_FAN_RED),
	LED_INFO(REAR_FAN1_RED),
	LED_INFO(REAR_FAN2_RED),
	LED_INFO(REAR_FAN3_RED),
	LED_INFO(REAR_FAN4_RED),
};

static struct gpio_led system_led[] = {
#if defined(QUANTA_CPU_RGL)
	{
		.name = "sysled_amber",
		.default_trigger = "timer",
		.gpio = (PCA9555_CPU_LED_GPIO_BASE + 10),
		.active_low = 0,
	},
	{
		.name = "sysled_green",
		.gpio = (PCA9555_CPU_LED_GPIO_BASE + 11),
		.active_low = 0,
	},
#endif
	{
		.name = "front_led_psu1_green",
		.gpio = (PCA9555_PSU_GPIO_BASE + 10),
		.active_low = 0,
	},
	{
		.name = "front_led_psu1_red",
		.gpio = (PCA9555_PSU_GPIO_BASE + 11),
		.active_low = 0,
	},
	{
		.name = "front_led_psu2_green",
		.gpio = (PCA9555_PSU_GPIO_BASE + 12),
		.active_low = 0,
	},
	{
		.name = "front_led_psu2_red",
		.gpio = (PCA9555_PSU_GPIO_BASE + 13),
		.active_low = 0,
	},
	{
		.name = "front_led_fan_green",
		.gpio = (PCA9555_PSU_GPIO_BASE + 14),
		.active_low = 0,
	},
	{
		.name = "front_led_fan_red",
		.gpio = (PCA9555_PSU_GPIO_BASE + 15),
		.active_low = 0,
	},
	{
		.name = "rear_led_fan1_red",
		.gpio = (PCA9555_FAN_GPIO_BASE + 12),
		.active_low = 0,
	},
	{
		.name = "rear_led_fan2_red",
		.gpio = (PCA9555_FAN_GPIO_BASE + 13),
		.active_low = 0,
	},
	{
		.name = "rear_led_fan3_red",
		.gpio = (PCA9555_FAN_GPIO_BASE + 14),
		.active_low = 0,
	},
	{
		.name = "rear_led_fan4_red",
		.gpio = (PCA9555_FAN_GPIO_BASE + 15),
		.active_low = 0,
	},
};

static struct gpio_led_platform_data system_led_data = {
	.num_leds	= ARRAY_SIZE(system_led),
	.leds		= system_led
};

static struct platform_device system_led_dev = {
	.name	= "leds-gpio",
	.id		= -1,
	.dev	= {
		.platform_data	= &system_led_data,
	},
};

static struct platform_device *ix1b_device;

struct gpio_led_data {
	struct led_classdev cdev;
	unsigned gpio;
	struct work_struct work;
	u8 new_level;
	u8 can_sleep;
	u8 active_low;
	u8 blinking;
	int (*platform_gpio_blink_set)(unsigned gpio, int state,
	unsigned long *delay_on, unsigned long *delay_off);
};

struct gpio_leds_priv {
	int num_leds;
	struct gpio_led_data leds[];
};

static struct i2c_adapter *i2c_get_adapter_wait(int nr)
{
	struct i2c_adapter *adap = NULL;
	int i = 0;

	for (i = 0; i < 300; ++i) {
		adap = i2c_get_adapter(nr);
		if (adap)
			break;
		msleep(10);
	}

	if (adap == NULL)
		printk(KERN_ERR "%s: unable to get i2c adapter for bus %d\n", __FILE__, nr);

	return adap;
}

static int __init ix1b_platform_init(void)
{
	struct i2c_client *client;
	struct i2c_adapter *adapter;
	struct gpio_leds_priv *priv;
	int i;
	int ret;

	ret = platform_driver_register(&ix1b_platform_driver);
	if (ret < 0)
		return ret;

	/* Register platform stuff */
	ix1b_device = platform_device_alloc("qci-ix1b", -1);
	if (!ix1b_device) {
		ret = -ENOMEM;
		goto fail_platform_driver;
	}

	ret = platform_device_add(ix1b_device);
	if (ret)
		goto fail_platform_device;

	adapter = i2c_get_adapter_wait(0);
	client = i2c_new_device(adapter, &ix1b_i2c_devices[0]);		// pca9546_1
	printk("[CC] NEW device pca9546_1\n");
	client = i2c_new_device(adapter, &ix1b_i2c_devices[1]);		// pca9546_2
	printk("[CC] NEW device pca9546_2\n");
#if defined(QUANTA_CPU_RGL)
	client = i2c_new_device(adapter, &ix1b_i2c_devices[14]);		// cpu pca9546_1
	printk("[CC] NEW device cpu pca9546_1\n");
#endif
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter_wait(0x14);
	client = i2c_new_device(adapter, &ix1b_i2c_devices[12]);		// pmbus_psu1
	printk("[CC] NEW device pmbus_psu1\n");
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter_wait(0x15);
	client = i2c_new_device(adapter, &ix1b_i2c_devices[13]);		// pmbus_psu2
	printk("[CC] NEW device pmbus_psu2\n");
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter_wait(0x16);
	client = i2c_new_device(adapter, &ix1b_i2c_devices[2]);		// pca9555-PSU1
	printk("[CC] NEW device pca9555-PSU1\n");
	client = i2c_new_device(adapter, &ix1b_i2c_devices[3]);		// MB_BOARDINFO_EEPROM
	printk("[CC] NEW device MB_BOARDINFO_EEPROM\n");
	client = i2c_new_device(adapter, &ix1b_i2c_devices[10]);		// pca9555-ID
	printk("[CC] NEW device pca9555-ID\n");
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter_wait(0x17);
	client = i2c_new_device(adapter, &ix1b_i2c_devices[11]);		// pca9555-fan
	printk("[CC] NEW device pca9555-fan\n");
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter_wait(0x10);
	client = i2c_new_device(adapter, &ix1b_i2c_devices[4]);		// pca9548_2 SFP
	printk("[CC] NEW device pca9548_2 SFP\n");
	client = i2c_new_device(adapter, &ix1b_i2c_devices[5]);		// pca9548_3 SFP
	printk("[CC] NEW device pca9548_3 SFP\n");
	client = i2c_new_device(adapter, &ix1b_i2c_devices[6]);		// pca9548_4 SFP
	printk("[CC] NEW device pca9548_4 SFP\n");
	client = i2c_new_device(adapter, &ix1b_i2c_devices[8]);		// CPLD2
	printk("[CC] NEW device CPLD2\n");
	client = i2c_new_device(adapter, &ix1b_i2c_devices[9]);		// CPLD3
	printk("[CC] NEW device CPLD3\n");
	i2c_put_adapter(adapter);

	adapter = i2c_get_adapter_wait(0x11);
	client = i2c_new_device(adapter, &ix1b_i2c_devices[7]);		// pca9548_5 SFP
	printk("[CC] NEW device pca9548_5 SFP\n");
	i2c_put_adapter(adapter);

#if defined(QUANTA_CPU_RGL)
	adapter = i2c_get_adapter_wait(0x18);
	client = i2c_new_device(adapter, &ix1b_i2c_devices[15]);		// cpu pca9555_1
	printk("[CC] NEW device cpu pca9555_1\n");
	i2c_put_adapter(adapter);

	for(i = 0x20; i < 0x40; i++)
	{
		adapter = i2c_get_adapter_wait(i);
		client = i2c_new_device(adapter, &ix1b_i2c_devices[16]);		// eeprom for loopback module
		i2c_put_adapter(adapter);
	}
	printk("[CC] NEW device eeprom\n");
#endif

	/* Register LED devices */
	platform_device_register(&system_led_dev);
	priv = platform_get_drvdata(&system_led_dev);
	for (i = 0; i < priv->num_leds; i++) {
		printk("leds-gpio gpio-%d: Register led %s\n", priv->leds[i].gpio, priv->leds[i].cdev.name);
		ix1b_leds[i].cdev = &(priv->leds[i].cdev);
	}
	printk("[CC] NEW device led\n");

	return 0;

fail_platform_device:
	platform_device_put(ix1b_device);

fail_platform_driver:
	platform_driver_unregister(&ix1b_platform_driver);
	return ret;
}

static void __exit ix1b_platform_exit(void)
{
	platform_device_unregister(ix1b_device);
	platform_driver_unregister(&ix1b_platform_driver);
}

module_init(ix1b_platform_init);
module_exit(ix1b_platform_exit);


MODULE_AUTHOR("Quanta Computer Inc.");
MODULE_DESCRIPTION("Quanta IX1B Platform Driver");
MODULE_LICENSE("GPL");


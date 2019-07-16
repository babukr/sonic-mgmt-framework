/*
 * Copyright 2019 Broadcom. All rights reserved.
 * The term “Broadcom” refers to Broadcom Inc. and/or its subsidiaries.
 *
 * Description
 *  Platform LED related defines and structures
 */


/*****************************************
 *  kobj list 
 *****************************************/

struct kobject *platform_kobj=NULL;
struct kobject *led_kobj=NULL;

struct kobject *blink_kobj=NULL;
struct kobject *on_kobj=NULL;
struct kobject *off_kobj=NULL;
struct kobject *faulty_kobj=NULL;
struct kobject *cur_state_kobj=NULL;

/*****************************************
 * Static Data provided from user 
 * space JSON data file
 *****************************************/
#define NAME_SIZE 32
typedef enum {
	ON,
	OFF,
	FAULTY,
	BLINK,
        MAX_LED_STATUS		
}LED_STATUS;
typedef struct 
{
    char bits[NAME_SIZE]; 
    int pos; 
    int mask_bits;
}MASK_BITS;

typedef struct
{
    int	swpld_addr;
    int swpld_addr_offset;
    char color[NAME_SIZE];
    MASK_BITS bits;
    unsigned short	value;
} LED_DATA;

typedef struct
{
    int state;
    char color[NAME_SIZE]; 
    char color_state[NAME_SIZE]; //BLINK or SOLID
} CUR_STATE_DATA;

typedef struct
{
    CUR_STATE_DATA cur_state;
    char device_name[NAME_SIZE];
    int index;
    LED_DATA data[MAX_LED_STATUS];
    bool blink;
    int	swpld_addr;
    int swpld_addr_offset;
} LED_OPS_DATA; 

typedef enum{
	LED_SYS,
	LED_PSU,
	LED_FAN,
	LED_FANTRAY,
	LED_DIAG,
	LED_LOC,
	LED_TYPE_MAX
} LED_TYPE;
char* LED_TYPE_STR[LED_TYPE_MAX] = 
{
	"LED_SYS",
	"LED_PSU",
	"LED_FAN",
	"LED_FANTRAY",
	"LED_DIAG",
	"LED_LOC",
};

/*****************************************
 * Data exported from kernel for  
 * user space plugin to get/set
 *****************************************/
#define PDDF_LED_DATA_ATTR( _prefix, _name, _mode, _show, _store, _type, _len, _addr)    \
        struct pddf_data_attribute pddf_dev_##_prefix##_attr_##_name = { .dev_attr = __ATTR(_name, _mode, _show, _store),  \
          .type = _type , \
          .len = _len ,   \
          .addr = _addr }

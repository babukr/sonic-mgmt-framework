/*
 * Copyright 2019 Broadcom. All rights reserved.
 * The term “Broadcom” refers to Broadcom Inc. and/or its subsidiaries.
 *
 * Description
 *  Optics driver related api declarations
 */
#ifndef __PDDF_XCVR_API_H__
#define __PDDF_XCVR_API_H__

extern ssize_t get_module_presence(struct device *dev, struct device_attribute *da, char *buf);
extern ssize_t get_module_reset(struct device *dev, struct device_attribute *da, char *buf);
extern ssize_t set_module_reset(struct device *dev, struct device_attribute *da, const char *buf, size_t count);
extern ssize_t get_module_intr_status(struct device *dev, struct device_attribute *da, char *buf);
extern ssize_t get_module_lpmode(struct device *dev, struct device_attribute *da, char *buf);
extern ssize_t set_module_lpmode(struct device *dev, struct device_attribute *da, const char *buf, size_t count);
extern ssize_t get_module_rxlos(struct device *dev, struct device_attribute *da, char *buf);
extern ssize_t get_module_txdisable(struct device *dev, struct device_attribute *da, char *buf);
extern ssize_t set_module_txdisable(struct device *dev, struct device_attribute *da, const char *buf, size_t count);
extern ssize_t get_module_txfault(struct device *dev, struct device_attribute *da, char *buf);

#endif //__PDDF_XCVR_API_H__

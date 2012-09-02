/*
 * Copyright 2012 Federico Vaga <federico.vaga@gmail.com>
 */

#ifndef ZIO_DEVICE_H_
#define ZIO_DEVICE_H_

#include <stdint.h>
#include <linux/zio-user.h>
#include "libsysfs.h"

#define ZIO_DIR "/sys/bus/zio"
#define ZIO_DIR_DEV ZIO_DIR"/devices"
#define ZIO_TRG_LIST ZIO_DIR
#define ZIO_BUF_LIST ZIO_DIR

extern const char *zio_dir;
extern const char *zio_dev_dir;

struct zio_object {
	unsigned int n_attr;	/* number of attributes in the list*/
	char *attribute[];	/* list of path to sysfs attributes */
};

/* libzattr.c */
extern int zio_read_attribute(struct sysfs_attr *attr, uint32_t *value);
extern int zio_write_attribute(struct sysfs_attr *attr, uint32_t value);
extern int zio_read_control(struct sysfs_attr *attr, struct zio_control *ctrl);
extern int zio_write_control(struct sysfs_attr *attr, struct zio_control *ctrl);
extern int zio_get_module(struct sysfs_attr *attr, char *name);
extern int zio_set_module(struct sysfs_attr *attr, char *name);

#endif /* ZIO_DEVICE_H_ */

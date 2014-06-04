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

enum zio_attr_type {
	ZIO_ATTR_TYPE_STD,
	ZIO_ATTR_TYPE_EXT,
	ZIO_ATTR_TYPE_PAR,
};

struct zio_attr {
	struct sysfs_attr *attribute; /* list of sysfs attributes */
	enum zio_attr_type type;
};

struct zio_attr_set {
	unsigned int n_attr; /* number of attributes in the list */
	struct zio_attr *attr;
};

struct zio_bi {
	struct zio_attr_set set;

	struct sysfs_dir *dir;
};

struct zio_ti {
	struct zio_attr_set set;

	struct sysfs_dir *dir;
};


struct zio_channel {
	char name[32]; /* FIXME probably less than 32 */
	char devpath[32];
	struct zio_attr_set set;

	struct zio_bi buf;

	struct sysfs_dir *dir;
};

struct zio_cset {
	char name[32]; /* FIXME probably less than 32 */
	char devpath[32];
	struct zio_attr_set set;

	struct zio_ti trg;

	struct zio_channel *chan;
	unsigned int n_chan;

	struct sysfs_dir *dir;
};

struct zio_device {
	char name[32]; /* FIXME probably less than 32 */
	char devpath[32];
	struct zio_attr_set set;

	struct zio_cset *cset;
	unsigned int n_cset;

	struct sysfs_dir *dir;
};



struct zio_object {
	unsigned int n_attr;	/* number of attributes in the list*/
	char *attribute[];	/* list of path to sysfs attributes */
};

/* libzattr.c */
extern int zio_attr_read(struct sysfs_attr *attr, uint32_t *value);
extern int zio_attr_write(struct sysfs_attr *attr, uint32_t value);
extern int zio_ctrl_read(struct sysfs_attr *attr, struct zio_control *ctrl);
extern int zio_ctrl_write(struct sysfs_attr *attr, struct zio_control *ctrl);
extern int zio_get_module(struct sysfs_attr *attr, char *name);
extern int zio_set_module(struct sysfs_attr *attr, char *name);

/* libzio.c */
extern void zio_device_destroy(struct zio_device *dev);
extern struct zio_device *zio_device_create(char *path);

#ifdef _LIBZIO_INTERNAL_



#endif

#endif /* ZIO_DEVICE_H_ */

/*
 * Copyright (c) 2012 Federico Vaga
 * Author: Federico Vaga <federico.vaga@gmail.com>
 * License: GPL v3
 */

#ifndef ZIO_DEVICE_H_
#define ZIO_DEVICE_H_

#include <stdint.h>
#include <linux/zio-user.h>

#define ZIO_DIR "/sys/bus/zio"
#define ZIO_DIR_DEV ZIO_DIR"/devices"
#define ZIO_TRG_LIST ZIO_DIR
#define ZIO_BUF_LIST ZIO_DIR
#define UZIO_MAX_PATH_LEN 128
#define UZIO_NAME_MAX_LEN 32

enum uzio_errno {
	EUZIONODEV = 19860705,
	EUZIOVERSION,
};

extern const char *zio_dir;
extern const char *zio_dev_dir;

#define UZIO_FLAG_INTERLEAVE (1 << 0)


enum zio_object_type {
	ZIO_NONE = 0,   /* reserved for non zio object */
	ZIO_DEV, ZIO_CSET, ZIO_CHAN,
	ZIO_TRG, ZIO_TI, /* trigger and trigger instance */
	ZIO_BUF, ZIO_BI, /* buffer and buffer instance */
};

enum zio_attr_type {
	ZIO_ATTR_TYPE_STD,
	ZIO_ATTR_TYPE_EXT,
	ZIO_ATTR_TYPE_PAR,
};


struct zio_attr {
	char *name;		/* file name */
	char path[UZIO_MAX_PATH_LEN];		/* full path to the attribute */
	mode_t md;		/* protection option */
	unsigned int index;     /* index inside the control */
	enum zio_attr_type type;
};

struct zio_attr_set {
	unsigned int n_attr; /* number of attributes in the list */
	struct zio_attr *attr;
};


struct uzio_ti {
	char *name;
	char path[UZIO_MAX_PATH_LEN];

	struct zio_attr_set uzattr_set;
};

struct uzio_bi {
	char name[UZIO_NAME_MAX_LEN];
	char path[UZIO_MAX_PATH_LEN];

	struct zio_attr_set uzattr_set;
};

struct uzio_channel{
	char name[UZIO_NAME_MAX_LEN];
	char path[UZIO_MAX_PATH_LEN];

	struct uzio_bi bi;

	unsigned long flags;

	struct zio_attr_set uzattr_set;
};

struct uzio_cset {
	char name[UZIO_NAME_MAX_LEN];
	char path[UZIO_MAX_PATH_LEN];

	struct uzio_channel *uchan_inter;
	struct uzio_channel *uchan;
	unsigned int n_uchan;

	char ti_type[UZIO_NAME_MAX_LEN];
	char bi_type[UZIO_NAME_MAX_LEN];

	struct uzio_ti ti;

	unsigned long flags;

	struct zio_attr_set uzattr_set;
};

struct uzio_device {
	char *name;
	char path[UZIO_MAX_PATH_LEN];

	struct uzio_cset *ucset;
	unsigned int n_ucset;

	unsigned long flags;

	struct zio_attr_set uzattr_set;
};


/* libzattr.c */
extern int uzio_attr_read(struct zio_attr *attr, uint32_t *value);
extern int uzio_attr_write(struct zio_attr *attr, uint32_t value);
extern int uzio_ctrl_read(struct uzio_chan *chan, struct zio_control *ctrl);
extern int uzio_ctrl_write(struct uzio_chan *chan, struct zio_control *ctrl);
/*extern int uzio_get_module(struct zio_attr *attr, char *name);
  extern int uzio_set_module(struct zio_attr *attr, char *name);*/

/* libzio.c */
extern void uzio_device_destroy(struct uzio_device *dev);
extern struct uzio_device *uzio_device_create(char *name);

#ifdef _LIBZIO_INTERNAL_

extern int _uzio_attr_read_raw(char *path, void *buf, size_t len);
extern int _uzio_attr_write_raw(char *path, void *buf, size_t len);
#endif

#endif /* ZIO_DEVICE_H_ */

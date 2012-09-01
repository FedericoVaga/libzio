/*
 * Copyright 2012 Federico Vaga <federico.vaga@gmail.com>
 */

#ifndef ZIO_DEVICE_H_
#define ZIO_DEVICE_H_

#include <stdint.h>
#include <linux/zio-user.h>

#define ZIO_DIR "/sys/bus/zio"
#define ZIO_DIR_DEV ZIO_DIR"/devices"
#define ZIO_TRG_LIST ZIO_DIR
#define ZIO_BUF_LIST ZIO_DIR

extern const char *zio_dir;
extern const char *zio_dev_dir;


/* libzattr.c */
extern int zio_read_attribute(char *path, uint32_t *value);
extern int zio_write_attribute(char *path, uint32_t value);
extern inline int zio_read_control(char *path, struct zio_control *ctrl);
extern inline int zio_write_control(char *path, struct zio_control *ctrl);


#endif /* ZIO_DEVICE_H_ */

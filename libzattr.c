/*
 * Copyright (c) 2012 Federico Vaga
 * Author: Federico Vaga <federico.vaga@gmail.com>
 * License: GPL v3
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

#include <linux/zio-user.h>

#include "libzio.h"

#define ZATTR_BUF_LEN 50

int _uzio_attr_read_raw(char *path, void *buf, size_t len)
{
	int fd, i;
	char *str;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	i = read(fd, buf, len);
	close(fd);
	str = buf;
	if (str[i - 1] == '\n')
		str[i - 1] = '\0';
	return i;
}

int _uzio_attr_write_raw(char *path, void *buf, size_t len)
{
	int fd, i;

	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	i = read(fd, buf, len);
	close(fd);
	return i;
}

/* The function read the value of the ZIO sysfs attribute */
int uzio_attr_read(struct zio_attr *attr, uint32_t *value)
{
	char buf[ZATTR_BUF_LEN];
	int i;

	if (!(attr->md & (S_IRUSR | S_IRGRP | S_IROTH))) {
		errno = EPERM;
		return -1;
	}

	i = _uzio_attr_read_raw(attr->path, buf, ZATTR_BUF_LEN);
	/* Read attribute */
	if (i <=0 )
		return -1;
	/* Convert an ASCII string to long */
	sscanf(buf, "%i", value);

	return 0;
}
/* Set the value to the zio attribute */
int uzio_attr_write(struct zio_attr *attr, uint32_t value)
{
	char buf[ZATTR_BUF_LEN];
	int i;

	if (!(attr->md & (S_IWUSR | S_IWGRP | S_IWOTH))) {
		errno = EPERM;
		return -1;
	}

	/* Convert the value to an ASCII string */
	sprintf(buf, "%d", value);
	/* Write attribute */
	i = _uzio_attr_write_raw(attr->path, buf, ZATTR_BUF_LEN);
	if (i)
		return -1;

	return 0;
}

/* Generic read/write for the current control attribute */
static int __zio_ctrl_rw(struct zio_attr *attr, struct zio_control * ctrl, int flags)
{
	int i, err = 0;

	/* Only current_control attribute can use this function */
	if (strcmp(attr->name, "current_control"))
		return -1;
	if (flags == O_RDONLY)
		i = _uzio_attr_read_raw(attr->path, ctrl, __ZIO_CONTROL_SIZE);
	else
		i = _uzio_attr_write_raw(attr->path, ctrl, __ZIO_CONTROL_SIZE);
	/* Check what happen during file I/O */
	switch (i) {
	case -1:
		fprintf(stderr, "File I/O err: %s\n", strerror(errno));
		err = -EIO;
		break;
	case 0:
		fprintf(stderr, "File I/O err: unexpected EOF\n");
		err = -EIO;
		break;
	default: /* FIXME usefull only for control */
		fprintf(stderr, "File I/O warn: %i bytes (expected %i)\n",
			i, __ZIO_CONTROL_SIZE);
		/* continue anyways */
	case __ZIO_CONTROL_SIZE:
		break; /* ok */
	}
	return err;
}
/* Get the current control of a channel from the binary sysfs attribute */
int uzio_ctrl_read(struct zio_attr *attr, struct zio_control *ctrl)
{
	return __zio_ctrl_rw(attr, ctrl, O_RDONLY);
}
/* Set the current control of a channel to the binary sysfs attribute */
int zio_ctrl_write(struct zio_attr *attr, struct zio_control *ctrl)
{

	return __zio_ctrl_rw(attr, ctrl, O_WRONLY);
}


/* These function are used to handle the modules to use within a device */
/*int uzio_get_module(struct zio_attr *attr, char *name)
{
	if (strcmp(attr->name, "current_trigger") &&
	    strcmp(attr->name, "current_buffer"))
		return -1;
	return _uzio_attr_read_raw(attr->path, name, ZATTR_BUF_LEN);
}
int uzio_set_module(struct zio_attr *attr, char *name)
{
	if (strcmp(attr->name, "current_trigger") &&
	    strcmp(attr->name, "current_buffer"))
		return -1;
	return _uzio_attr_write_raw(attr->path, name, strlen(name));
	}*/

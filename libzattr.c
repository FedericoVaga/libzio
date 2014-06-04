
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
#include "libsysfs.h"

#define ZATTR_BUF_LEN 50

/* The function read the value of the ZIO sysfs attribute */
int zio_attr_read(struct sysfs_attr *attr, uint32_t *value)
{
	char buf[ZATTR_BUF_LEN];
	int i;

	i = sysfs_read_attribute(attr, buf, ZATTR_BUF_LEN);
	/* Read attribute */
	if (i <=0 )
		return -1;
	/* Convert an ASCII string to long */
	sscanf(buf, "%i", value);

	return 0;
}
/* Set the value to the zio attribute */
int zio_attr_write(struct sysfs_attr *attr, uint32_t value)
{
	char buf[ZATTR_BUF_LEN];
	int i;

	/* Convert the value to an ASCII string */
	sprintf(buf, "%d", value);
	/* Write attribute */
	i = sysfs_write_attribute(attr, buf, ZATTR_BUF_LEN);
	if (i)
		return -1;

	return 0;
}

/* Generic read/write for the current control attribute */
static int __zio_ctrl_rw(struct sysfs_attr *attr, struct zio_control * ctrl, int flags)
{
	int i, err = 0;

	/* Only current_control attribute can use this function */
	if (strcmp(attr->name, "current_control"))
		return -1;
	if (flags == O_RDONLY)
		i = sysfs_read_attribute(attr, ctrl, ZIO_CONTROL_SIZE);
	else
		i = sysfs_write_attribute(attr, ctrl, ZIO_CONTROL_SIZE);
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
			i, ZIO_CONTROL_SIZE);
		/* continue anyways */
	case ZIO_CONTROL_SIZE:
		break; /* ok */
	}
	return err;
}
/* Get the current control of a channel from the binary sysfs attribute */
int zio_ctrl_read(struct sysfs_attr *attr, struct zio_control *ctrl)
{
	return __zio_ctrl_rw(attr, ctrl, O_RDONLY);
}
/* Set the current control of a channel to the binary sysfs attribute */
int zio_ctrl_write(struct sysfs_attr *attr, struct zio_control *ctrl)
{

	return __zio_ctrl_rw(attr, ctrl, O_WRONLY);
}


/* These function are used to handle the modules to use within a device */
int zio_get_module(struct sysfs_attr *attr, char *name)
{
	if (strcmp(attr->name, "current_trigger") &&
	    strcmp(attr->name, "current_buffer"))
		return -1;
	return sysfs_read_attribute(attr, name, ZATTR_BUF_LEN);
}
int zio_set_module(struct sysfs_attr *attr, char *name)
{
	if (strcmp(attr->name, "current_trigger") &&
	    strcmp(attr->name, "current_buffer"))
		return -1;
	return sysfs_write_attribute(attr, name, strlen(name));
}

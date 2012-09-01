
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

/*
 * These function is used to read/write from a ZIO attribute. It can be used
 * both to read normal and binary sysfs attributes. The function return 0 on
 * success or a negative error if it fail.
 *
 * @fullpath: full path to the sysfs attribute
 * @flags: the flags to use to open the file
 * @buf: buffer to write or read (it depends on flags)
 * @len: buffer size in bytes
 */
static int __zio_rw_attribute(char *fullpath, int flags, void *buf, size_t len)
{
	int fd, i, err;

	/* Open the sysfs attribute */
	fd = open(fullpath, flags);
	if (fd < 0) {
		fprintf(stderr, "%s: %s\n", fullpath, strerror(errno));
		return -ENOENT;
	}
	/* Sysfs file I/O */
	if (flags == O_WRONLY)
		i = write(fd, buf, strlen(buf));
	else
		i = read(fd, buf, len);
	/* Close the sysfs attribute */
	close(fd);

	return (i < 0 ? i : 0);
}


#define ZATTR_BUF_LEN 50

/* The function read the value of the ZIO sysfs attribute */
int zio_read_attribute(char *path, uint32_t *value)
{
	char buf[ZATTR_BUF_LEN];
	int err;

	/* Read attribute */
	err = __zio_rw_attribute(path, O_RDONLY, buf, ZATTR_BUF_LEN);
	if (err)
		return err;

	/* Convert an ASCII string to long */
	*value = (uint32_t)strtol(buf, NULL, 0);

	return 0;
}
/* Set the value to the zio attribute */
int zio_write_attribute(char *path, uint32_t value)
{
	char buf[ZATTR_BUF_LEN];
	int err;

	/* Convert the value to an ASCII string */
	sprintf(buf, "%d", value);
	/* Write attribute */
	err = __zio_rw_attribute(path, O_WRONLY, buf, ZATTR_BUF_LEN);
	if (err)
		return err;

	return 0;
}

/*
 * The following functions can be used both to read the current_contorl
 * attribute within each channel, and the control char device from the cdev
 * interface. If you need a complex management of the cdev control do not use
 * these functions; for example if you need to use select() or poll() you must
 * handle the char device differently.
 */
static int __zio_rw_control(char *path, struct zio_control * ctrl, int flags)
{
	int i, err = 0;

	i = __zio_rw_attribute(path, flags, ctrl, ZIO_CONTROL_SIZE);
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
inline int zio_read_control(char *path, struct zio_control *ctrl)
{
	return __zio_rw_control(path, ctrl, O_RDONLY);

}
/* Set the current control of a channel to the binary sysfs attribute */
inline int zio_write_control(char *path, struct zio_control *ctrl)
{
	return __zio_rw_control(path, ctrl, O_WRONLY);
}


/*
 * libsysfs.h
 *
 *  Created on: 02/set/2012
 *      Author: federico
 */

#ifndef LIBSYSFS_H_
#define LIBSYSFS_H_

#include <sys/types.h>

#define ENOZDEV 210000

struct sysfs_attr {
	char *name;		/* file name */
	char *path;		/* full path to the attribute */

	mode_t md;		/* protection option */
};

struct sysfs_dir {
	char *name;			/* directory name */
	char *path;			/* full path to directory */
	/* Attribute */
	unsigned int n_attr;		/* number of attributes in the list*/
	struct sysfs_attr **attr;	/* list of sysfs attributes */
	/* Sub Directory */
	unsigned int n_sub_dir;		/* number of sub directory */
	struct sysfs_dir **sub_dir;	/* sub directory */
};

/* Read and write a sysfs attribute */
extern int sysfs_read_attribute(struct sysfs_attr *attr, void *buf, size_t len);
extern int sysfs_write_attribute(struct sysfs_attr *attr, void *buf, size_t len);

/* Build the sysfs tree from a particular point */
extern void sysfs_destroy_directory_tree(struct sysfs_dir *dir);
extern struct sysfs_dir *sysfs_build_directory_tree(char *pathto, char *name);

#endif /* LIBSYSFS_H_ */

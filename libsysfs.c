/*
 * Copyright 2012 Federico Vaga <federico.vaga@gmail.com>
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
#include <sys/types.h>

#include "libsysfs.h"


int sysfs_read_attribute(struct sysfs_attr *attr, void *buf, size_t len)
{
	int fd, i;

	if (!(attr->md & (S_IRUSR | S_IRGRP | S_IROTH))) {
		errno = EPERM;
		return -1;
	}
	/* Open the sysfs attribute */
	fd = open(attr->path, O_RDONLY);
	if (fd < 0)
		return -1;
	i = read(fd, buf, len);
	/* Close the sysfs attribute */
	close(fd);
	return i;
}
int sysfs_write_attribute(struct sysfs_attr *attr, void *buf, size_t len)
{
	int fd, i;

	if (!(attr->md & (S_IWUSR | S_IWGRP | S_IWOTH))) {
		errno = EPERM;
		return -1;
	}
	/* Open the sysfs attribute */
	fd = open(attr->path, O_WRONLY);
	if (fd < 0)
		return -1;
	i = read(fd, buf, len);
	/* Close the sysfs attribute */
	close(fd);
	return i;
}


/* Destroy a sysfs_attr*/
static void _sysfs_destroy_attr(struct sysfs_attr *attr)
{
	free(attr->name);
	free(attr->path);
	free(attr);
}
/* Create and initialize a sysfs_attr */
static struct sysfs_attr *_sysfs_create_attr(struct sysfs_dir *dir,
					     char *name, mode_t md)
{
	struct sysfs_attr *attr;

	attr = malloc(sizeof(struct sysfs_attr));
	if (!attr)
		return NULL;
	/* Copy name */
	attr->name = malloc(strlen(name));
	if (!attr->name)
		goto out;
	strcpy(attr->name, name);

	/* Copy path ( +1 for the slash +1 for terminator) */
	attr->path = malloc(strlen(attr->name) + strlen(dir->path) + 2);
	if (!attr->name)
		goto out;
	sprintf(attr->path, "%s/%s", dir->path, attr->name);

	attr->md = md;
	return attr;
out:
	free(attr);
	return NULL;
}

/* Destroy a directory tree */
void sysfs_destroy_directory_tree(struct sysfs_dir *dir)
{
	int i;

	for (i = 0; i < dir->n_attr; ++i)
		_sysfs_destroy_attr(dir->attr[i]);
	for (i = 0; i < dir->n_sub_dir; ++i)
		sysfs_destroy_directory_tree(dir->sub_dir[i]);

	free(dir->sub_dir);
	free(dir->attr);
	free(dir->path);
	free(dir->name);
	free(dir);
}
/* Build a directory tree */
struct sysfs_dir *sysfs_build_directory_tree(char *pathto, char *name)
{
	struct sysfs_dir *dir;
	struct dirent *ep, **namelist;
	struct stat st;
	int n, i, a, d;

	dir = malloc(sizeof(struct sysfs_dir));
	if (!dir)
		return NULL;
	dir->name = malloc(strlen(name));
	if (!dir->name)
		goto out;
	strcpy(dir->name, name);
	dir->path = malloc(strlen(pathto) + strlen(name) + 2);
	if (!dir->path)
		goto out_path;
	sprintf(dir->path, "%s/%s", pathto, name);

	n = scandir(dir->path, &namelist, 0, alphasort);
	if (n < 0)
		goto out_scan;
	/* Count attributes and directory */
	for (i = 0; i < n; ++i) {
		stat(namelist[i]->d_name, &st);
		if (IS_DIR(st.st_mode)) {
			if (IS_LINK(st.st_mode))
				continue;
			dir->n_sub_dir++;
		} else {
			dir->n_attr++;
		}
	}
	/* Allocate */
	dir->attr = malloc(sizeof(struct sysfs_attr *) * dir->n_attr);
	if (!dir->attr)
		goto out_attr;
	dir->sub_dir = malloc(sizeof(struct sysfs_dir *) * dir->n_attr);
	if (!dir->sub_dir)
		goto out_dir;
	/* Fill directory */
	for (i = 0, a = 0, d = 0; i < n; ++i) {
		stat(namelist[i]->d_name, &st);
		if (IS_DIR(st.st_mode)) {
			if (IS_LINK(st.st_mode))
				continue;
			dir->sub_dir[d] = sysfs_build_directory_tree(dir->path,
							  namelist[i]->d_name);
			if (!dir->sub_dir[d])
				goto out_fill;
			d++;
		} else {
			dir->attr[a] = _sysfs_create_attr(dir,
							  namelist[i]->d_name,
							  st.st_mode);
			if (!dir->attr[a])
				goto out_fill;
			a++;
		}
	}

	for (i = 0; i < n; ++i)
		free(namelist[i]);
	free(namelist);

	return dir;

out_fill:
	while(a--)
		_sysfs_destroy_attr(dir->attr[a]);
	while(d--)
		free(dir->sub_dir[d]);

	free(dir->sub_dir);
out_dir:
	free(dir->attr);
out_attr:
	for (i = 0; i < n; ++i)
		free(namelist[i]);
	free(namelist);
out_scan:
	free(dir->path);
out_path:
	free(dir->name);
out:
	free(dir);
	return NULL;
}

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
#include <sys/types.h>
#include <sys/stat.h>

#define _LIBZIO_INTERNAL_ 1
#include "libzio.h"

const char *zio_dir = ZIO_DIR;
const char *zio_dev_dir = ZIO_DIR_DEV;

const char *zio_obj_type[] = {
	[ZIO_DEV] = "zio_zdev_type",
	[ZIO_CSET] = "zio_cset_type",
	[ZIO_CHAN] = "zio_chan_type",
	[ZIO_TI] = "zio_ti_type",
	[ZIO_BI] = "zio_bi_type",
};

int uzio_cset_trg_type_update(struct uzio_cset *ucset)
{
	char path[UZIO_MAX_PATH_LEN];

	/* Get trigger type */
	snprintf(path, UZIO_MAX_PATH_LEN, "%s/current_trigger", ucset->path);
	_uzio_attr_read_raw(path, ucset->ti_type, UZIO_NAME_MAX_LEN);

	return 0;
}

int uzio_cset_buf_type_update(struct uzio_cset *ucset)
{
	char path[UZIO_MAX_PATH_LEN];

	/* Get buffer type */
	snprintf(path, UZIO_MAX_PATH_LEN, "%s/current_buffer", ucset->path);
	_uzio_attr_read_raw(path, ucset->bi_type, UZIO_NAME_MAX_LEN);

	return 0;
}

static int __zio_obj_get_devpath(char *obj_path, char *devname)
{
	char path[128], tmp[64];
	int err;

	printf("%s %d\n", __func__, __LINE__);
	sprintf(path, "%s/devname", obj_path);
	err = _uzio_attr_read_raw(path, tmp, 128);
	if (err < 0)
		return -1;
	printf("%s %d\n", __func__, __LINE__);
	sprintf(devname, "%s/%s", ZIO_DEV, tmp);
	return 0;
}


static int is_zio_object(char *obj_path, enum zio_object_type type)
{
	char path[128], tmp[64];
	int err, found = 0;

	sprintf(path, "%s/devtype", obj_path);
	err = _uzio_attr_read_raw(path, tmp, 128);
	if (err < 0)
		return -1;
	if (strcmp(tmp, zio_obj_type[type]))
		return 0;
	return 1;
}



static int _uzio_alloc_attr(FILE *f, struct zio_attr_set *uzattr_set)
{
	fscanf(f, "%d", &uzattr_set->n_attr);

	if (uzattr_set->n_attr == 0)
		return 0;

	uzattr_set->attr = calloc(uzattr_set->n_attr, sizeof(struct zio_attr));
	if (!uzattr_set->attr)
		return -1;

	return 0;
}

static void  _uzio_attr_free(struct zio_attr_set *uzattr_set)
{
	if (uzattr_set->n_attr == 0)
		return;

	free(uzattr_set->attr);
}

static int _uzio_fill_attr(FILE *f, struct zio_attr_set *zattr_set)
{
	char relpath[128], type;
	int i;

	for (i = 0; i < zattr_set->n_attr; ++i) {
		fscanf(f, "%s %c %d %d", relpath, &type,
		       &zattr_set->attr[i].index, &zattr_set->attr[i].md);

		switch (type) {
		case 'p':
			zattr_set->attr[i].type = ZIO_ATTR_TYPE_PAR;
			break;
		case 's':
			zattr_set->attr[i].type = ZIO_ATTR_TYPE_STD;
			break;
		case 'e':
			zattr_set->attr[i].type = ZIO_ATTR_TYPE_EXT;
			break;
		}

		snprintf(zattr_set->attr[i].path, UZIO_MAX_PATH_LEN ,"%s/%s",
			zio_dev_dir, relpath);
		zattr_set->attr[i].name = basename(zattr_set->attr[i].path);
	}

	return 0;
}

struct uzio_device *uzio_device_create(char *name)
{
	struct uzio_device *uzdev = NULL;
	struct stat st;
	char tmp[64], path[UZIO_MAX_PATH_LEN];
	int err, i, k, y;
	char type;
	FILE *f;

	/* a valid ZIO device path must exist and it must be a directory */
	sprintf(path, "%s/%s", zio_dev_dir, name);
	err = stat(path, &st);
	if (err < 0)
		goto out_stat;
	if (!S_ISDIR(st.st_mode)) {
		errno = EUZIONODEV;
		goto out_stat;
	}

	/* this library support only ZIO version with device-description attribute */
	sprintf(tmp, "%s/device-description", path);
	err = stat(tmp, &st);
	if (err < 0)
		goto out_stat;
	if (!S_ISREG(st.st_mode)) {
		errno = EUZIOVERSION;
		goto out_stat;
	}


	/* Create the ZIO hierarchy */
	uzdev = malloc(sizeof(struct uzio_device));
	if (!uzdev)
		goto out_alloc_dv;

	strncpy(uzdev->path, path, UZIO_MAX_PATH_LEN);
	uzdev->name = basename(uzdev->path);
	f = fopen(tmp, "r");
	if (!f)
		goto out_open;


	/* Find and allocate cset */
	fscanf(f, "%d", &uzdev->n_ucset);
	uzdev->ucset = calloc(uzdev->n_ucset, sizeof(struct uzio_cset));
	if (!uzdev->ucset)
		goto out_alloc_cs;

	for (i = 0; i < uzdev->n_ucset; ++i) {
		struct uzio_cset *ucset = &uzdev->ucset[i];

		/* Set CSET path and name */
		snprintf(ucset->path, UZIO_MAX_PATH_LEN, "%s/cset%d",
			 uzdev->path, i);
		snprintf(path, UZIO_MAX_PATH_LEN, "%s/name", ucset->path);
		err = _uzio_attr_read_raw(path, ucset->name, UZIO_NAME_MAX_LEN);
		if (err < 0)
			goto out_read_cs_name;

		/* Set Trigger path */
		snprintf(ucset->ti.path, UZIO_MAX_PATH_LEN, "%s/trigger",
			 ucset->path);

		/* Set Trigger and Buffer type */
		err = uzio_cset_trg_type_update(ucset);
		if (err)
			goto out_trg_type;

		err = uzio_cset_buf_type_update(ucset);
		if (err)
			goto out_buf_type;

		/* Allocate channel */
		fscanf(f, "%d %c", &ucset->n_uchan, &type);

		if (type == 'i') {
			ucset->flags |= UZIO_FLAG_INTERLEAVE;
			ucset->n_uchan--;
			ucset->uchan_inter = malloc(sizeof(struct uzio_channel));
			if (!ucset->uchan_inter)
				goto out_alloc_it;
			snprintf(ucset->uchan_inter->path, UZIO_MAX_PATH_LEN,
				 "%s/chani", ucset->path);
			strcpy(ucset->uchan_inter->name, "chani");
		}

		ucset->uchan = calloc(ucset->n_uchan,
				      sizeof(struct uzio_channel));
		if (!ucset->uchan) {
			if (type == 'i')
				free(ucset->uchan_inter);
			goto out_alloc_ch;
		}

		for (k = 0; k < ucset->n_uchan; ++k) {
			struct uzio_channel *uchan = &ucset->uchan[k];

			/* Set CHAN path and name */
			snprintf(uchan->path, UZIO_MAX_PATH_LEN, "%s/chan%d",
				 ucset->path, k);
			snprintf(path, UZIO_MAX_PATH_LEN, "%s/name", uchan->path);
			err = _uzio_attr_read_raw(path, uchan->name,
						  UZIO_NAME_MAX_LEN);
			if (err < 0)
				goto out_read_ch_name;


			/* Set Buffer path */
			snprintf(uchan->bi.path, UZIO_MAX_PATH_LEN, "%s/buffer",
			  uchan->path);
		}

	}

	/* Find all attribute counters and allocate attribute structure */
	err = _uzio_alloc_attr(f, &uzdev->uzattr_set);
	if (err)
		goto out_alloc_dva;
	for (i = 0; i < uzdev->n_ucset; ++i) {
		struct uzio_cset *ucset = &uzdev->ucset[i];

		err = _uzio_alloc_attr(f, &ucset->uzattr_set);
		if (err)
			goto out_alloc_csa;

		err = _uzio_alloc_attr(f, &ucset->ti.uzattr_set);
		if (err)
			goto out_alloc_tia;


		for (k = 0; k < ucset->n_uchan; ++k) {
			struct uzio_channel *uchan = &ucset->uchan[k];

			err = _uzio_alloc_attr(f, &uchan->uzattr_set);
			if (err)
				goto out_alloc_cha;

			err = _uzio_alloc_attr(f, &uchan->bi.uzattr_set);
			if (err)
				goto out_alloc_bia;
		}
	}

	/* Looks for attributes */
	_uzio_fill_attr(f, &uzdev->uzattr_set);
	for (i = 0; i < uzdev->n_ucset; ++i) {
		struct uzio_cset *ucset = &uzdev->ucset[i];

		_uzio_fill_attr(f, &ucset->uzattr_set);
		_uzio_fill_attr(f, &ucset->ti.uzattr_set);
		for (k = 0; k < ucset->n_uchan; ++k) {
			struct uzio_channel *uchan = &ucset->uchan[k];

			_uzio_fill_attr(f, &uchan->uzattr_set);
			_uzio_fill_attr(f, &uchan->bi.uzattr_set);
		}
	}

	fclose(f);

	return uzdev;

out_alloc_bia:
out_alloc_cha:
out_alloc_tia:
out_alloc_csa:
	while (--i >= 0) {
		while (--k >= 0) {
			free(uzdev->ucset[i].uchan[k].bi.uzattr_set.attr);
			free(uzdev->ucset[i].uchan[k].uzattr_set.attr);
		}
		free(uzdev->ucset[i].ti.uzattr_set.attr);
		free(uzdev->ucset[i].uzattr_set.attr);

	}
	free(uzdev->uzattr_set.attr);
	i = uzdev->n_ucset;
out_read_ch_name:
out_alloc_dva:
out_alloc_it:
out_alloc_ch:
out_buf_type:
out_trg_type:
out_read_cs_name:
	while (--i >= 0) {
		free(uzdev->ucset[i].uchan);
		if (uzdev->ucset[i].uchan_inter)
			free(uzdev->ucset[i].uchan_inter);
	}
	free(uzdev->ucset);
out_alloc_cs:
	fclose(f);
out_open:
	free(uzdev);
out_alloc_dv:
out_stat:
	return NULL;
}


void uzio_device_destroy(struct uzio_device *uzdev)
{
	int i, k;

	for (i = 0; i < uzdev->n_ucset; ++i) {
	        for (k = 0; k < uzdev->ucset[i].n_uchan; ++k) {
			 _uzio_attr_free(&uzdev->ucset[i].uchan[k].bi.uzattr_set);
			 _uzio_attr_free(&uzdev->ucset[i].uchan[k].uzattr_set);
		}
		_uzio_attr_free(&uzdev->ucset[i].ti.uzattr_set);
		_uzio_attr_free(&uzdev->ucset[i].uzattr_set);

	}
	_uzio_attr_free(&uzdev->uzattr_set);
	for (i = 0; i < uzdev->n_ucset; ++i) {
		free(uzdev->ucset[i].uchan);
		if (uzdev->ucset[i].uchan_inter)
			free(uzdev->ucset[i].uchan_inter);
	}
	free(uzdev->ucset);
	free(uzdev);
}

// SPDX-License-Identifier: LGPL-2.1
// Copyright (C) 2014-2020, Intel Corporation. All rights reserved.
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <dirent.h>
#include <libkmod.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <util/log.h>
#include <util/sysfs.h>

int __sysfs_read_attr(struct log_ctx *ctx, const char *path, char *buf)
{
	int fd = open(path, O_RDONLY|O_CLOEXEC);
	int n;

	if (fd < 0) {
		log_dbg(ctx, "failed to open %s: %s\n", path, strerror(errno));
		return -errno;
	}
	n = read(fd, buf, SYSFS_ATTR_SIZE);
	close(fd);
	if (n < 0 || n >= SYSFS_ATTR_SIZE) {
		buf[0] = 0;
		log_dbg(ctx, "failed to read %s: %s\n", path, strerror(errno));
		return -errno;
	}
	buf[n] = 0;
	if (n && buf[n-1] == '\n')
		buf[n-1] = 0;
	return 0;
}

static int write_attr(struct log_ctx *ctx, const char *path,
		const char *buf, int quiet)
{
	int fd = open(path, O_WRONLY|O_CLOEXEC);
	int n, len = strlen(buf) + 1, rc;

	if (fd < 0) {
		rc = -errno;
		log_dbg(ctx, "failed to open %s: %s\n", path, strerror(errno));
		return rc;
	}
	n = write(fd, buf, len);
	rc = -errno;
	close(fd);
	if (n < len) {
		if (!quiet)
			log_dbg(ctx, "failed to write %s to %s: %s\n", buf, path,
					strerror(errno));
		return rc;
	}
	return 0;
}

int __sysfs_write_attr(struct log_ctx *ctx, const char *path,
		const char *buf)
{
	return write_attr(ctx, path, buf, 0);
}

int __sysfs_write_attr_quiet(struct log_ctx *ctx, const char *path,
		const char *buf)
{
	return write_attr(ctx, path, buf, 1);
}

int __sysfs_device_parse(struct log_ctx *ctx, const char *base_path,
		const char *dev_name, void *parent, add_dev_fn add_dev)
{
	int add_errors = 0;
	struct dirent *de;
	DIR *dir;

	log_dbg(ctx, "base: '%s' dev: '%s'\n", base_path, dev_name);
	dir = opendir(base_path);
	if (!dir) {
		log_dbg(ctx, "no \"%s\" devices found\n", dev_name);
		return -ENODEV;
	}

	while ((de = readdir(dir)) != NULL) {
		char *dev_path;
		char fmt[20];
		void *dev;
		int id;

		sprintf(fmt, "%s%%d", dev_name);
		if (de->d_ino == 0)
			continue;
		if (sscanf(de->d_name, fmt, &id) != 1)
			continue;
		if (asprintf(&dev_path, "%s/%s", base_path, de->d_name) < 0) {
			log_err(ctx, "%s%d: path allocation failure\n",
					dev_name, id);
			continue;
		}

		dev = add_dev(parent, id, dev_path);
		free(dev_path);
		if (!dev) {
			add_errors++;
			log_err(ctx, "%s%d: add_dev() failed\n",
					dev_name, id);
		} else
			log_dbg(ctx, "%s%d: processed\n", dev_name, id);
	}
	closedir(dir);

	return add_errors;
}

struct kmod_module *__util_modalias_to_module(struct kmod_ctx *kmod_ctx,
					      const char *alias,
					      struct log_ctx *log)
{
	struct kmod_list *list = NULL;
	struct kmod_module *mod;
	int rc;

	if (!kmod_ctx)
		return NULL;

	rc = kmod_module_new_from_lookup(kmod_ctx, alias, &list);
	if (rc < 0 || !list) {
		log_dbg(log,
			"failed to find module for alias: %s %d list: %s\n",
			alias, rc, list ? "populated" : "empty");
		return NULL;
	}
	mod = kmod_module_get_module(list);
	log_dbg(log, "alias: %s module: %s\n", alias,
		kmod_module_get_name(mod));
	kmod_module_unref_list(list);

	return mod;
}

int __util_bind(const char *devname, struct kmod_module *module,
		const char *bus, struct log_ctx *ctx)
{
	DIR *dir;
	int rc = 0;
	char path[200];
	struct dirent *de;
	const int len = sizeof(path);

	if (!devname) {
		log_err(ctx, "missing devname\n");
		return -EINVAL;
	}

	if (module) {
		rc = kmod_module_probe_insert_module(module,
						     KMOD_PROBE_APPLY_BLACKLIST,
						     NULL, NULL, NULL, NULL);
		if (rc < 0) {
			log_err(ctx, "%s: insert failure: %d\n", __func__, rc);
			return rc;
		}
	}

	if (snprintf(path, len, "/sys/bus/%s/drivers", bus) >= len) {
		log_err(ctx, "%s: buffer too small!\n", devname);
		return -ENXIO;
	}

	dir = opendir(path);
	if (!dir) {
		log_err(ctx, "%s: opendir(\"%s\") failed\n", devname, path);
		return -ENXIO;
	}

	while ((de = readdir(dir)) != NULL) {
		char *drv_path;

		if (de->d_ino == 0)
			continue;
		if (de->d_name[0] == '.')
			continue;

		if (asprintf(&drv_path, "%s/%s/bind", path, de->d_name) < 0) {
			log_err(ctx, "%s: path allocation failure\n", devname);
			continue;
		}

		rc = __sysfs_write_attr_quiet(ctx, drv_path, devname);
		free(drv_path);
		if (rc == 0)
			break;
	}
	closedir(dir);

	if (rc) {
		log_dbg(ctx, "%s: bind failed\n", devname);
		return -ENXIO;
	}
	return 0;
}

int __util_unbind(const char *devpath, struct log_ctx *ctx)
{
	const char *devname = devpath_to_devname(devpath);
	char path[200];
	const int len = sizeof(path);

	if (snprintf(path, len, "%s/driver/unbind", devpath) >= len) {
		log_err(ctx, "%s: buffer too small!\n", devname);
		return -ENXIO;
	}

	return __sysfs_write_attr(ctx, path, devname);
}

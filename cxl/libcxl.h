/* SPDX-License-Identifier: LGPL-2.1 */
/* Copyright (C) 2020-2021, Intel Corporation. All rights reserved. */
#ifndef _LIBCXL_H_
#define _LIBCXL_H_

#include <stdarg.h>
#include <unistd.h>

#ifdef HAVE_UUID
#include <uuid/uuid.h>
#else
typedef unsigned char uuid_t[16];
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct cxl_ctx;
struct cxl_ctx *cxl_ref(struct cxl_ctx *ctx);
void cxl_unref(struct cxl_ctx *ctx);
int cxl_new(struct cxl_ctx **ctx);
void cxl_set_log_fn(struct cxl_ctx *ctx,
		void (*log_fn)(struct cxl_ctx *ctx, int priority,
			const char *file, int line, const char *fn,
			const char *format, va_list args));
int cxl_get_log_priority(struct cxl_ctx *ctx);
void cxl_set_log_priority(struct cxl_ctx *ctx, int priority);
void cxl_set_userdata(struct cxl_ctx *ctx, void *userdata);
void *cxl_get_userdata(struct cxl_ctx *ctx);
void cxl_set_private_data(struct cxl_ctx *ctx, void *data);
void *cxl_get_private_data(struct cxl_ctx *ctx);

struct cxl_memdev;
struct cxl_memdev *cxl_memdev_get_first(struct cxl_ctx *ctx);
struct cxl_memdev *cxl_memdev_get_next(struct cxl_memdev *memdev);
int cxl_memdev_get_id(struct cxl_memdev *memdev);
const char *cxl_memdev_get_devname(struct cxl_memdev *memdev);
int cxl_memdev_get_major(struct cxl_memdev *memdev);
int cxl_memdev_get_minor(struct cxl_memdev *memdev);
struct cxl_ctx *cxl_memdev_get_ctx(struct cxl_memdev *memdev);
unsigned long long cxl_memdev_get_pmem_size(struct cxl_memdev *memdev);
unsigned long long cxl_memdev_get_ram_size(struct cxl_memdev *memdev);
const char *cxl_memdev_get_firmware_verison(struct cxl_memdev *memdev);
size_t cxl_memdev_get_label_size(struct cxl_memdev *memdev);
int cxl_memdev_nvdimm_bridge_active(struct cxl_memdev *memdev);
int cxl_memdev_zero_label(struct cxl_memdev *memdev, size_t length,
		size_t offset);
int cxl_memdev_read_label(struct cxl_memdev *memdev, void *buf, size_t length,
		size_t offset);
int cxl_memdev_write_label(struct cxl_memdev *memdev, void *buf, size_t length,
		size_t offset);

#define cxl_memdev_foreach(ctx, memdev) \
        for (memdev = cxl_memdev_get_first(ctx); \
             memdev != NULL; \
             memdev = cxl_memdev_get_next(memdev))

struct cxl_cmd;
const char *cxl_cmd_get_devname(struct cxl_cmd *cmd);
struct cxl_cmd *cxl_cmd_new_raw(struct cxl_memdev *memdev, int opcode);
int cxl_cmd_set_input_payload(struct cxl_cmd *cmd, void *in, int size);
int cxl_cmd_set_output_payload(struct cxl_cmd *cmd, void *out, int size);
void cxl_cmd_ref(struct cxl_cmd *cmd);
void cxl_cmd_unref(struct cxl_cmd *cmd);
int cxl_cmd_submit(struct cxl_cmd *cmd);
int cxl_cmd_get_mbox_status(struct cxl_cmd *cmd);
int cxl_cmd_get_out_size(struct cxl_cmd *cmd);
struct cxl_cmd *cxl_cmd_new_identify(struct cxl_memdev *memdev);
int cxl_cmd_identify_get_fw_rev(struct cxl_cmd *cmd, char *fw_rev, int fw_len);
unsigned long long cxl_cmd_identify_get_partition_align(struct cxl_cmd *cmd);
unsigned int cxl_cmd_identify_get_label_size(struct cxl_cmd *cmd);
struct cxl_cmd *cxl_cmd_new_get_health_info(struct cxl_memdev *memdev);
int cxl_cmd_health_info_get_maintenance_needed(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_performance_degraded(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_hw_replacement_needed(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_normal(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_not_ready(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_persistence_lost(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_data_lost(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_normal(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_not_ready(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_persistence_lost(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_data_lost(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_powerloss_persistence_loss(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_shutdown_persistence_loss(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_persistence_loss_imminent(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_powerloss_data_loss(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_shutdown_data_loss(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_media_data_loss_imminent(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_ext_life_used_normal(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_ext_life_used_warning(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_ext_life_used_critical(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_ext_temperature_normal(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_ext_temperature_warning(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_ext_temperature_critical(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_ext_corrected_volatile_normal(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_ext_corrected_volatile_warning(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_ext_corrected_persistent_normal(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_ext_corrected_persistent_warning(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_life_used(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_temperature(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_dirty_shutdowns(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_volatile_errors(struct cxl_cmd *cmd);
int cxl_cmd_health_info_get_pmem_errors(struct cxl_cmd *cmd);
struct cxl_cmd *cxl_cmd_new_read_label(struct cxl_memdev *memdev,
		unsigned int offset, unsigned int length);
ssize_t cxl_cmd_read_label_get_payload(struct cxl_cmd *cmd, void *buf,
		unsigned int length);
struct cxl_cmd *cxl_cmd_new_write_label(struct cxl_memdev *memdev,
		void *buf, unsigned int offset, unsigned int length);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

/* SPDX-License-Identifier: LGPL-2.1 */
/* Copyright (C) 2020-2021, Intel Corporation. All rights reserved. */
#ifndef _LIBCXL_PRIVATE_H_
#define _LIBCXL_PRIVATE_H_

#include <libkmod.h>
#include <cxl/cxl_mem.h>
#include <ccan/endian/endian.h>
#include <ccan/short_types/short_types.h>

#define CXL_EXPORT __attribute__ ((visibility("default")))

struct cxl_nvdimm_bridge {
	int id;
	void *dev_buf;
	size_t buf_len;
	char *dev_path;
};

struct cxl_memdev {
	int id, major, minor;
	void *dev_buf;
	size_t buf_len;
	char *dev_path;
	char *firmware_version;
	struct cxl_ctx *ctx;
	struct list_node list;
	unsigned long long pmem_size;
	unsigned long long ram_size;
	int payload_max;
	size_t lsa_size;
	struct kmod_module *module;
	struct cxl_nvdimm_bridge *bridge;
};

enum cxl_cmd_query_status {
	CXL_CMD_QUERY_NOT_RUN = 0,
	CXL_CMD_QUERY_OK,
	CXL_CMD_QUERY_UNSUPPORTED,
};

/**
 * struct cxl_cmd - CXL memdev command
 * @memdev: the memory device to which the command is being sent
 * @query_cmd: structure for the Linux 'Query commands' ioctl
 * @send_cmd: structure for the Linux 'Send command' ioctl
 * @input_payload: buffer for input payload managed by libcxl
 * @output_payload: buffer for output payload managed by libcxl
 * @refcount: reference for passing command buffer around
 * @query_status: status from query_commands
 * @query_idx: index of 'this' command in the query_commands array
 * @status: command return status from the device
 */
struct cxl_cmd {
	struct cxl_memdev *memdev;
	struct cxl_mem_query_commands *query_cmd;
	struct cxl_send_command *send_cmd;
	void *input_payload;
	void *output_payload;
	int refcount;
	int query_status;
	int query_idx;
	int status;
};

#define CXL_CMD_IDENTIFY_FW_REV_LENGTH 0x10

struct cxl_cmd_identify {
	char fw_revision[CXL_CMD_IDENTIFY_FW_REV_LENGTH];
	le64 total_capacity;
	le64 volatile_capacity;
	le64 persistent_capacity;
	le64 partition_align;
	le16 info_event_log_size;
	le16 warning_event_log_size;
	le16 failure_event_log_size;
	le16 fatal_event_log_size;
	le32 lsa_size;
	u8 poison_list_max_mer[3];
	le16 inject_poison_limit;
	u8 poison_caps;
	u8 qos_telemetry_caps;
} __attribute__((packed));

struct cxl_cmd_get_lsa_in {
	le32 offset;
	le32 length;
} __attribute__((packed));

struct cxl_cmd_set_lsa {
	le32 offset;
	le32 rsvd;
	unsigned char lsa_data[0];
} __attribute__ ((packed));

struct cxl_cmd_get_health_info {
	u8 health_status;
	u8 media_status;
	u8 ext_status;
	u8 life_used;
	le16 temperature;
	le32 dirty_shutdowns;
	le32 volatile_errors;
	le32 pmem_errors;
} __attribute__((packed));

/* CXL 2.0 8.2.9.5.3 Byte 0 Health Status */
#define CXL_CMD_HEALTH_INFO_STATUS_MAINTENANCE_NEEDED_MASK		BIT(0)
#define CXL_CMD_HEALTH_INFO_STATUS_PERFORMANCE_DEGRADED_MASK		BIT(1)
#define CXL_CMD_HEALTH_INFO_STATUS_HW_REPLACEMENT_NEEDED_MASK		BIT(2)

/* CXL 2.0 8.2.9.5.3 Byte 1 Media Status */
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_NORMAL				0x0
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_NOT_READY			0x1
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_PERSISTENCE_LOST		0x2
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_DATA_LOST			0x3
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_POWERLOSS_PERSISTENCE_LOSS	0x4
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_SHUTDOWN_PERSISTENCE_LOSS	0x5
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_PERSISTENCE_LOSS_IMMINENT	0x6
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_POWERLOSS_DATA_LOSS		0x7
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_SHUTDOWN_DATA_LOSS		0x8
#define CXL_CMD_HEALTH_INFO_MEDIA_STATUS_DATA_LOSS_IMMINENT		0x9

/* CXL 2.0 8.2.9.5.3 Byte 2 Additional Status */
#define CXL_CMD_HEALTH_INFO_EXT_LIFE_USED_MASK				GENMASK(1, 0)
#define CXL_CMD_HEALTH_INFO_EXT_LIFE_USED_NORMAL			(0)
#define CXL_CMD_HEALTH_INFO_EXT_LIFE_USED_WARNING			(1)
#define CXL_CMD_HEALTH_INFO_EXT_LIFE_USED_CRITICAL			(2)
#define CXL_CMD_HEALTH_INFO_EXT_TEMPERATURE_MASK			GENMASK(3, 2)
#define CXL_CMD_HEALTH_INFO_EXT_TEMPERATURE_NORMAL			(0)
#define CXL_CMD_HEALTH_INFO_EXT_TEMPERATURE_WARNING			(1)
#define CXL_CMD_HEALTH_INFO_EXT_TEMPERATURE_CRITICAL			(2)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_VOLATILE_MASK			BIT(4)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_VOLATILE_NORMAL		(0)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_VOLATILE_WARNING		(1)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_PERSISTENT_MASK		BIT(5)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_PERSISTENT_NORMAL		(0)
#define CXL_CMD_HEALTH_INFO_EXT_CORRECTED_PERSISTENT_WARNING		(1)

#define CXL_CMD_HEALTH_INFO_LIFE_USED_NOT_IMPL				0xff
#define CXL_CMD_HEALTH_INFO_TEMPERATURE_NOT_IMPL			0xffff

static inline int check_kmod(struct kmod_ctx *kmod_ctx)
{
	return kmod_ctx ? 0 : -ENXIO;
}

#endif /* _LIBCXL_PRIVATE_H_ */

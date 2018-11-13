/*
 * wprobe.h: API for the wireless probe interface
 * Copyright (C) 2008-2009 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __WPROBE_H
#define __WPROBE_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/if_ether.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/list.h>
#include <net/genetlink.h>
#endif

/** 
 * enum wprobe_attr: netlink attribute list
 *
 * @WPROBE_ATTR_UNSPEC: unused
 *
 * @WPROBE_ATTR_INTERFACE: interface name to process query on (NLA_STRING)
 * @WPROBE_ATTR_MAC: mac address (used for wireless links) (NLA_STRING)
 * @WPROBE_ATTR_FLAGS: interface/link/attribute flags (see enum wprobe_flags) (NLA_U32)
 * @WPROBE_ATTR_DURATION: sampling duration (in milliseconds) (NLA_MSECS)
 *
 * @WPROBE_ATTR_ID: attribute id (NLA_U32)
 * @WPROBE_ATTR_NAME: attribute name (NLA_STRING)
 * @WPROBE_ATTR_TYPE: attribute type (NLA_U8)
 * @WPROBE_ATTR_SCALE: attribute scale factor (NLA_U32)
 *
 * attribute values:
 *
 * @WPROBE_VAL_STRING: string value (NLA_STRING)
 * @WPROBE_VAL_S8: signed 8-bit integer (NLA_U8)
 * @WPROBE_VAL_S16: signed 16-bit integer (NLA_U16)
 * @WPROBE_VAL_S32: signed 32-bit integer (NLA_U32)
 * @WPROBE_VAL_S64: signed 64-bit integer (NLA_U64)
 * @WPROBE_VAL_U8: unsigned 8-bit integer (NLA_U8)
 * @WPROBE_VAL_U16: unsigned 16-bit integer (NLA_U16)
 * @WPROBE_VAL_U32: unsigned 32-bit integer (NLA_U32)
 * @WPROBE_VAL_U64: unsigned 64-bit integer (NLA_U64)
 *
 * statistics:
 * @WPROBE_VAL_SUM: sum of all samples
 * @WPROBE_VAL_SUM_SQ: sum of all samples^2
 * @WPROBE_VAL_SAMPLES: number of samples
 *
 * @WPROBE_ATTR_LAST: unused
 */
enum wprobe_attr {
	WPROBE_ATTR_UNSPEC,
	WPROBE_ATTR_INTERFACE,
	WPROBE_ATTR_MAC,
	WPROBE_ATTR_FLAGS,
	WPROBE_ATTR_DURATION,
	WPROBE_ATTR_SCALE,
	/* end of query attributes */

	/* response data */
	WPROBE_ATTR_ID,
	WPROBE_ATTR_NAME,
	WPROBE_ATTR_TYPE,

	/* value type attributes */
	WPROBE_VAL_STRING,
	WPROBE_VAL_S8,
	WPROBE_VAL_S16,
	WPROBE_VAL_S32,
	WPROBE_VAL_S64,
	WPROBE_VAL_U8,
	WPROBE_VAL_U16,
	WPROBE_VAL_U32,
	WPROBE_VAL_U64,

	/* aggregates for statistics */
	WPROBE_VAL_SUM,
	WPROBE_VAL_SUM_SQ,
	WPROBE_VAL_SAMPLES,

	WPROBE_ATTR_LAST
};


/**
 * enum wprobe_cmd: netlink commands for interacting with wprobe
 *
 * @WPROBE_CMD_UNSPEC: unused
 *
 * @WPROBE_CMD_GET_LIST: get global/link property list
 * @WPROBE_CMD_GET_INFO: get global/link properties
 * @WPROBE_CMD_SET_FLAGS: set global/link flags
 * @WPROBE_CMD_MEASURE: take a snapshot of the current data
 * @WPROBE_CMD_GET_LINKS: get a list of links
 *
 * @WPROBE_CMD_LAST: unused
 * 
 * options for GET_INFO and SET_FLAGS:
 *   - mac address set: per-link
 *   - mac address unset: globalsa
 */
enum wprobe_cmd {
	WPROBE_CMD_UNSPEC,
	WPROBE_CMD_GET_LIST,
	WPROBE_CMD_GET_INFO,
	WPROBE_CMD_SET_FLAGS,
	WPROBE_CMD_MEASURE,
	WPROBE_CMD_GET_LINKS,
	WPROBE_CMD_LAST
};

/**
 * enum wprobe_flags: flags for wprobe links and items
 * @WPROBE_F_KEEPSTAT: keep statistics for this link/device
 * @WPROBE_F_RESET: reset statistics now (used only in WPROBE_CMD_SET_LINK)
 * @WPROBE_F_NEWDATA: used to indicate that a value has been updated
 */
enum wprobe_flags {
	WPROBE_F_KEEPSTAT = (1 << 0),
	WPROBE_F_RESET = (1 << 1),
	WPROBE_F_NEWDATA = (1 << 2),
};

#ifdef __KERNEL__

struct wprobe_link;
struct wprobe_item;
struct wprobe_source;

/**
 * struct wprobe_link - data structure describing a wireless link
 * @iface: pointer to the wprobe_iface that this link belongs to
 * @addr: BSSID of the remote link partner
 * @flags: link flags (see wprobe_flags)
 * @priv: user pointer
 *
 * @list: for internal use
 * @val: for internal use
 */
struct wprobe_link {
	struct list_head list;
	struct wprobe_iface *iface;
	char addr[ETH_ALEN];
	u32 flags;
	void *priv;
	void *val;
};

/** 
 * struct wprobe_item - data structure describing the format of wprobe_link::data or wprobe_iface::data
 * @name: name of the field
 * @type: data type of this field
 * @flags: measurement item flags (see wprobe_flags)
 */
struct wprobe_item {
	const char *name;
	enum wprobe_attr type;
	u32 flags;
};

struct wprobe_value {
	bool pending;
	union {
		/*
		 * the following are kept uppercase to allow
		 * for automated checking against WPROBE_VAL_*
		 * via BUG_ON()
		 */
		const char *STRING;
		u8 U8;
		u16 U16;
		u32 U32;
		u64 U64;
		s8 S8;
		s16 S16;
		s32 S32;
		s64 S64;
	};
	s64 s, ss;
	unsigned int n;

	/* timestamps */
	u64 first, last;
};

/**
 * struct wprobe_source - data structure describing a wireless interface
 *
 * @name: name of the interface
 * @addr: local mac address of the interface
 * @links: list of wireless links to poll
 * @link_items: description of the per-link data structure
 * @n_link_items: number of link description items
 * @global_items: description of the per-interface data structure
 * @n_global_items: number of per-interface description items
 * @sync_data: callback allowing the driver to prepare data for the wprobe poll
 *
 * @list: head for the list of interfaces
 * @priv: user pointer
 * @lock: spinlock protecting value data access
 * @val: internal use
 * @query_val: internal use
 *
 * if sync_data is NULL, wprobe assumes that it can access the data structure
 * at any time (in atomic context). if sync_data returns a negative error code,
 * the poll request will not be handled for the given link
 */
struct wprobe_iface {
	/* to be filled in by wprobe source drivers */
	const char *name;
	const char *addr;
	const struct wprobe_item *link_items;
	int n_link_items;
	const struct wprobe_item *global_items;
	int n_global_items;

	int (*sync_data)(struct wprobe_iface *dev, struct wprobe_link *l, struct wprobe_value *val, bool measure);
	void *priv;

	/* handled by the wprobe core */
	struct list_head list;
	struct list_head links;
	spinlock_t lock;
	void *val;
	void *query_val;
};

#define WPROBE_FILL_BEGIN(_ptr, _list) do {			\
	struct wprobe_value *__val = (_ptr);			\
	const struct wprobe_item *__item = _list;		\
	u64 __msecs = jiffies_to_msecs(jiffies)

#define WPROBE_SET(_idx, _type, _value)				\
	if (__item[_idx].type != WPROBE_VAL_##_type) {		\
		printk("ERROR: invalid data type at %s:%d\n", __FILE__, __LINE__); \
		break;						\
	}							\
	__val[_idx].pending = true;				\
	__val[_idx]._type = _value;				\
	if (!__val[_idx].first)					\
		__val[_idx].first = __msecs;			\
	__val[_idx].first = __msecs

#define WPROBE_FILL_END()					\
} while(0)

/**
 * wprobe_add_iface: register an interface with the wireless probe subsystem
 * @dev: wprobe_iface structure describing the interface
 */
extern int __weak wprobe_add_iface(struct wprobe_iface *dev);

/**
 * wprobe_remove_iface: deregister an interface from the wireless probe subsystem
 * @dev: wprobe_iface structure describing the interface
 */
extern void __weak wprobe_remove_iface(struct wprobe_iface *dev);

/**
 * wprobe_add_link: register a new wireless link
 * @dev: wprobe_iface structure describing the interface
 * @l: storage space for the wprobe_link structure
 * @addr: mac address of the new link
 *
 * the entire wprobe_link structure is overwritten by this function call
 */
extern int __weak wprobe_add_link(struct wprobe_iface *dev, struct wprobe_link *l, const char *addr);

/**
 * wprobe_remove_link: deregister a previously registered wireless link
 * @dev: wprobe_iface structure describing the interface
 * @l: wprobe_link data structure
 */
extern void __weak wprobe_remove_link(struct wprobe_iface *dev, struct wprobe_link *l);

/**
 * wprobe_update_stats: update statistics after sampling values
 * @dev: wprobe_iface structure describing the interface
 * @l: wprobe_link data structure
 *
 * if l == NULL, then the stats for globals are updated
 */
extern void __weak wprobe_update_stats(struct wprobe_iface *dev, struct wprobe_link *l);

#endif /* __KERNEL__ */

#endif

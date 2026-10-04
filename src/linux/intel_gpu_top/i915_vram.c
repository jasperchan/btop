#include "i915_vram.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

/* --- minimal i915 uAPI subset, mirroring include/uapi/drm/i915_drm.h ------ */
/* DRM_IOCTL_I915_QUERY: DRM_COMMAND_BASE (0x40) + DRM_I915_QUERY (0x39). The
 * kernel's own _IOWR is used rather than open-coding the encoding. */
#define I915_VRAM_DRM_IOCTL_BASE 'd'
#define I915_VRAM_QUERY_NR 0x79

#define I915_VRAM_QUERY_MEMORY_REGIONS 4
#define I915_VRAM_MEMORY_CLASS_DEVICE  1

struct i915_vram_query_item {
	uint64_t query_id;
	int32_t length;
	uint32_t flags;
	uint64_t data_ptr;
};

struct i915_vram_query {
	uint32_t num_items;
	uint32_t flags;
	uint64_t items_ptr;
};

struct i915_vram_class_instance {
	uint16_t memory_class;
	uint16_t memory_instance;
};

struct i915_vram_region_info {
	struct i915_vram_class_instance region;
	uint32_t rsvd0;
	uint64_t probed_size;
	uint64_t unallocated_size;
	uint64_t rsvd1[8];
};

struct i915_vram_regions {
	uint32_t num_regions;
	uint32_t rsvd[3];
	struct i915_vram_region_info regions[];
};

#define I915_VRAM_IOCTL_QUERY \
	_IOWR(I915_VRAM_DRM_IOCTL_BASE, I915_VRAM_QUERY_NR, struct i915_vram_query)
/* ------------------------------------------------------------------------- */

int i915_vram_info(const char *drm_render_node, uint64_t *total, uint64_t *used) {
	if (!drm_render_node || !total || !used) return -1;
	*total = 0;
	*used = 0;

	const int fd = open(drm_render_node, O_RDWR | O_CLOEXEC);
	if (fd < 0) return -1;

	struct i915_vram_query_item item;
	struct i915_vram_query query;
	memset(&item, 0, sizeof(item));
	memset(&query, 0, sizeof(query));
	item.query_id = I915_VRAM_QUERY_MEMORY_REGIONS;
	query.num_items = 1;
	query.items_ptr = (uint64_t)(uintptr_t)&item;

	/* First pass asks the kernel how large the reply is. */
	if (ioctl(fd, I915_VRAM_IOCTL_QUERY, &query) || item.length <= 0) {
		close(fd);
		return -1;
	}

	struct i915_vram_regions *regions = calloc(1, (size_t)item.length);
	if (!regions) {
		close(fd);
		return -1;
	}
	item.data_ptr = (uint64_t)(uintptr_t)regions;

	int ret = -1;
	if (!ioctl(fd, I915_VRAM_IOCTL_QUERY, &query)) {
		for (uint32_t i = 0; i < regions->num_regions; i++) {
			const struct i915_vram_region_info *r = &regions->regions[i];
			if (r->region.memory_class != I915_VRAM_MEMORY_CLASS_DEVICE) continue;
			*total = r->probed_size;
			/* Without CAP_PERFMON the kernel reports everything as free. */
			*used = r->unallocated_size <= r->probed_size
				? r->probed_size - r->unallocated_size
				: 0;
			ret = 0;
			break;
		}
	}

	free(regions);
	close(fd);
	return ret;
}

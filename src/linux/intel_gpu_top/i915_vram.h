/* Minimal subset of i915 uAPI needed to query local (device) memory size and
 * usage. Declared here rather than including <drm/i915_drm.h> so the build does
 * not gain a libdrm dependency. Mirrors include/uapi/drm/i915_drm.h. */
#ifndef I915_VRAM_H
#define I915_VRAM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returns 0 on success and fills total/used in bytes.
 * Negative on failure. `used` requires CAP_PERFMON; without it the kernel
 * reports all memory as free and `used` is returned as 0. */
int i915_vram_info(const char *drm_render_node, uint64_t *total, uint64_t *used);

#ifdef __cplusplus
}
#endif

#endif

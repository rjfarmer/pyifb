/* SPDX-License-Identifier: GPL-2.0+ */
/* Helper shared library exporting a pre-initialised CFI_cdesc_t so that
 * tests can exercise CFI_cdesc.in_dll() without needing a Fortran source. */

#include "ISO_Fortran_binding.h"
#include <stdint.h>

static int32_t cdesc_data[3] = {10, 20, 30};

/* Exported rank-1 int32 descriptor.  The dynamic linker will relocate
 * base_addr to the run-time address of cdesc_data when the library loads. */
CFI_CDESC_T(1) cdesc_rank1 = {
    .base_addr = cdesc_data,
    .elem_len  = sizeof(int32_t),
    .version   = CFI_VERSION,
    .rank      = 1,
    .attribute = CFI_attribute_other,
    .type      = CFI_type_int,
    .dim       = {{ .lower_bound = 0, .extent = 3, .sm = sizeof(int32_t) }},
};

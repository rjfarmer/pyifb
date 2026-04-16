/* SPDX-License-Identifier: GPL-2.0+ */
/* Helper shared library exporting a pre-initialised CFI_cdesc_t so that
 * tests can exercise CFI_cdesc.in_dll() without needing a Fortran source. */

#include <ISO_Fortran_binding.h>
#include <stdint.h>

static int32_t cdesc_data[3] = {10, 20, 30};


CFI_CDESC_T(1) cdesc_rank1;


void setup() {
    const CFI_rank_t rank = 1;
    CFI_index_t lower[rank], upper[rank];
    int ind;
    int status;
    CFI_index_t extents[1] = {3};
    lower[0] = 1;
    upper[0] = 3;

    /* setup() is called once per test: release prior allocation if needed. */
    if (cdesc_rank1.base_addr != NULL) {
        (void)CFI_deallocate((CFI_cdesc_t *)&cdesc_rank1);
    }

    status = CFI_establish(
        (CFI_cdesc_t *)&cdesc_rank1,
        NULL,
        CFI_attribute_allocatable,
        CFI_type_int32_t,
        sizeof(int32_t),
        rank,
        extents
    );
    if (status != CFI_SUCCESS) {
        return;
    }

    status = CFI_allocate((CFI_cdesc_t *)&cdesc_rank1, lower, upper, sizeof(int32_t));
    if (status != CFI_SUCCESS) {
        return;
    }

    for (ind = 0; ind < 3; ind++) {
        ((int32_t *)cdesc_rank1.base_addr)[ind] = cdesc_data[ind];
    }
};
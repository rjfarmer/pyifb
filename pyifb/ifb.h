//SPDX-License-Identifier: GPL-2.0+

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "ISO_Fortran_binding.h"

#pragma once

typedef struct {
    PyObject_HEAD
    CFI_dim_t dim;
} CFI_dim_object;
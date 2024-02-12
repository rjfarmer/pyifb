//SPDX-License-Identifier: GPL-2.0+

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "ISO_Fortran_binding.h"

#pragma once

typedef struct {
    PyObject_HEAD
    CFI_dim_t dim;
} CFI_dim_object;


static int set_compiler(PyObject *m);
static int add_constants(PyObject *m);
static int add_compiler_constants(PyObject *m);
static int add_types(PyObject *m);
static int add_compiler_types(PyObject *m);
static int check_return(int error_code);
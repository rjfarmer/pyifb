//SPDX-License-Identifier: GPL-2.0+

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stddef.h>
#include "structmember.h"
#include "ISO_Fortran_binding.h"

#pragma once

typedef struct {
    PyObject_HEAD
    CFI_dim_t dim;
} CFI_dim_object;

typedef struct {
    PyObject_HEAD
    CFI_CDESC_T(CFI_MAX_RANK) dv;
} CFI_cdesc_object;


static int set_compiler(PyObject *m);
static int add_constants(PyObject *m);
static int add_compiler_constants(PyObject *m);
static int add_types(PyObject *m);
static int add_compiler_types(PyObject *m);
static int check_return(int error_code);

// Functions of CFI_dim_objects

// static PyObject* CFI_dim_getattro(CFI_dim_object *self, PyObject *attr);
// static int CFI_dim_setattro(CFI_dim_object *self, PyObject *attr, PyObject *value);

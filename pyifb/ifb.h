//SPDX-License-Identifier: GPL-2.0+

#define PY_SSIZE_T_CLEAN
#include <Python.h>

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
static PyMemberDef CFI_dim_members[] = {
    {"lower_bound",Py_T_INT,offsetof(CFI_dim_object,dim) + offsetof(CFI_dim_t,lower_bound),0,
        "The value is equal to the value of the lower bound for the dimension being described"},
    {"extent",Py_T_INT,offsetof(CFI_dim_object,dim) + offsetof(CFI_dim_t,extent),0,
        "The value is equal to the number of elements along the dimension being described, or the value -1 for the final dimension of an assumed-size array."},
    {"sm",Py_T_INT,offsetof(CFI_dim_object,dim) + offsetof(CFI_dim_t,sm),0,
        "The value is equal to the memory stride for a dimension. The value is the distance in bytes between the beginnings of successive elements along the dimension being described."},
    {NULL}  /* Sentinel */
};
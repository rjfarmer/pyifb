//SPDX-License-Identifier: GPL-2.0+

#pragma once
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h> // Removed in 3.12

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include "ISO_Fortran_binding.h"


// Stupid compiliers grabbing __GNUC__

#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)
#define REALLY_GCC   __GNUC__ 
#elif defined(__INTEL_COMPILER) 
#define REALLY_ICX __ __INTEL_COMPILER
#else
#define REALLY_UNKNOWN 
#endif


// Add fallback for compilier specific macros

#if defined(REALLY_ICX) || defined(REALLY_UNKNOWN)
#define CFI_FAILURE -1
#define CFI_INVALID_STRIDE -1
#define CFI_type_mask -1
#define CFI_type_kind_shift -1
#define CFI_type_Integer -1
#define CFI_type_Logical -1
#define CFI_type_Real -1
#define CFI_type_Complex -1
#define CFI_type_Character -1
#define CFI_type_cfunptr -1
#define CFI_type_float128 -1
#define CFI_type_float128_Complex -1
#endif

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)

#define PY_MAJOR  3
#define PY_STRUCT_SWITCH  12

#if PY_MAJOR_VERSION == PY_MAJOR
    #if PY_MINOR_VERSION < PY_STRUCT_SWITCH
        #define Py_T_INT T_INT
    #endif
#endif


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

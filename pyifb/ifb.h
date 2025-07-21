//SPDX-License-Identifier: GPL-2.0+

#pragma once
#define PY_SSIZE_T_CLEAN
#define Py_LIMITED_API 0x030A0000
#include <Python.h>

#if PY_MAJOR_VERSION == 3
    #if PY_MINOR_VERSION < 12
#include <structmember.h> // Removed in 3.12
    #endif
#endif

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

#if PY_MAJOR_VERSION == 3
    #if PY_MINOR_VERSION < 12
        #define Py_T_INT T_INT
        #define Py_T_LONG T_LONG
        #define Py_T_PYSSIZET T_PYSSIZET
        #define Py_T_SHORT T_SHORT
        #define Py_READONLY READONLY
    #endif
#endif

// Can't use sizeof here but we know that CFI_index_t is
// type ptrdiff_t
#if REALLY_GCC
#if __SIZEOF_PTRDIFF_T__ == 8
#define PyCFI_index_t Py_T_LONG
#else
#define PyCFI_index_t Py_T_INT
#endif

// int16
#define PyCFI_type_t Py_T_SHORT

#else
// Non gcc compilier will need thier own fix for 32 bit machines
// (if we care)
#define PyCFI_index_t Py_T_LONG
#endif

#define xstr(a) str(a)
#define str(a) #a

#define STR_CFI_MAX_RANK xstr(CFI_MAX_RANK)


static int set_compiler(PyObject *m);
static int add_constants(PyObject *m);
static int add_compiler_constants(PyObject *m);
static int add_types(PyObject *m);
static int add_compiler_types(PyObject *m);
static int add_pytypes(PyObject *m);

typedef struct {
    PyObject_HEAD
    CFI_dim_t dim;
} PyCFI_dim_object;

typedef struct {
    PyObject_VAR_HEAD
    CFI_cdesc_t dv;
} PyCFI_cdesc_object;



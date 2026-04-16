//SPDX-License-Identifier: GPL-2.0+

#pragma once
#define PY_SSIZE_T_CLEAN
#define Py_LIMITED_API 0x030A0000
#include <Python.h>
#include <structmember.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include <ISO_Fortran_binding.h>


// Stupid compilers grabbing __GNUC__

#if defined(__INTEL_LLVM_COMPILER) || defined(__INTEL_COMPILER)
    #define REALLY_ICX 1
#elif defined(__llvm__) || defined(__clang__)
    #define REALLY_LLVM 1
#elif defined(__GNUC__)
    #define REALLY_GCC 1
#else
    #define REALLY_UNKNOWN 1
#endif


// Add fallback for compiler specific macros

#if defined(REALLY_ICX) || defined(REALLY_LLVM)
    #ifndef CFI_type_float128
        #define CFI_type_float128 -1
    #endif
    #ifndef CFI_type_float128_Complex
        #define CFI_type_float128_Complex -1
    #endif

    #ifndef CFI_FAILURE
        #define CFI_FAILURE -1
    #endif
    #ifndef CFI_INVALID_STRIDE
        #define CFI_INVALID_STRIDE -1
    #endif
    #ifndef CFI_type_mask
        #define CFI_type_mask -1
    #endif
    #ifndef CFI_type_kind_shift
        #define CFI_type_kind_shift -1
    #endif
    #ifndef CFI_type_Integer
        #define CFI_type_Integer -1
    #endif
    #ifndef CFI_type_Logical
        #define CFI_type_Logical -1
    #endif
    #ifndef CFI_type_Real
        #define CFI_type_Real -1
    #endif
    #ifndef CFI_type_Complex
        #define CFI_type_Complex -1
    #endif
    #ifndef CFI_type_Character
        #define CFI_type_Character -1
    #endif
    #ifndef CFI_type_cfunptr
        #define CFI_type_cfunptr -1
    #endif
#endif

#ifndef Py_T_INT
    #define Py_T_INT T_INT
#endif
#ifndef Py_T_LONG
    #define Py_T_LONG T_LONG
#endif
#ifndef Py_T_PYSSIZET
    #define Py_T_PYSSIZET T_PYSSIZET
#endif
#ifndef Py_T_SHORT
    #define Py_T_SHORT T_SHORT
#endif
#ifndef Py_T_BYTE
    #define Py_T_BYTE T_BYTE
#endif
#ifndef Py_T_CHAR
    #define Py_T_CHAR T_CHAR
#endif
#ifndef Py_READONLY
    #define Py_READONLY READONLY
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

    // int8
    #define PyCFI_attribute_t Py_T_CHAR
    #define PyCFI_rank_t Py_T_CHAR

#elif REALLY_ICX
    #if __SIZEOF_PTRDIFF_T__ == 8
        #define PyCFI_index_t Py_T_LONG
        #define PyCFI_type_t Py_T_LONG
        #define PyCFI_attribute_t Py_T_LONG
        #define PyCFI_rank_t Py_T_LONG
    #else
        #define PyCFI_index_t Py_T_INT
        #define PyCFI_type_t Py_T_INT
        #define PyCFI_attribute_t Py_T_INT
        #define PyCFI_rank_t Py_T_INT
    #endif
#elif REALLY_LLVM
    #if __SIZEOF_PTRDIFF_T__ == 8
        #define PyCFI_index_t Py_T_LONG
    #else
        #define PyCFI_index_t Py_T_INT
    #endif
    #define PyCFI_type_t Py_T_BYTE
    #define PyCFI_attribute_t Py_T_BYTE
    #define PyCFI_rank_t Py_T_BYTE
#else
    #define PyCFI_index_t Py_T_LONG
    #define PyCFI_type_t Py_T_LONG
    #define PyCFI_attribute_t Py_T_LONG
    #define PyCFI_rank_t Py_T_LONG
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
    bool owns_memory;
    CFI_cdesc_t dv;
} PyCFI_cdesc_object;



static PyObject* new_PyCFI_dim();
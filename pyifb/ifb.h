//SPDX-License-Identifier: GPL-2.0+

#pragma once
#define PY_SSIZE_T_CLEAN
#define Py_LIMITED_API 3
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


static int set_compiler(PyObject *m);
static int add_constants(PyObject *m);
static int add_compiler_constants(PyObject *m);
static int add_types(PyObject *m);
static int add_compiler_types(PyObject *m);


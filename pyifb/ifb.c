// SPDX-License-Identifier: GPL-2.0+

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "ISO_Fortran_binding.h"

#include "ifb.h"

// https://github.com/gcc-mirror/gcc/blob/master/libgfortran/ISO_Fortran_binding.h
// https://wg5-fortran.org/N1901-N1950/N1942.pdf

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)


static PyTypeObject CFI_desc_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "CFI_desc_t",
    .tp_doc = PyDoc_STR("CFI_desc_t holds the dimensions of an object"),
    .tp_basicsize = sizeof(CFI_dim_object),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
};


PyModuleDef IFBModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "pyifb.ifb",
    .m_doc = PyDoc_STR("Bindings for ISO_Fortran_binding.h"),
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_ifb(void)
{

    PyObject *m;

    m = PyModule_Create(&IFBModule);
    if (m == NULL) {
        goto except;
    }

    if (PyType_Ready(&CFI_desc_type) < 0){
        goto except;
    }
        
    Py_INCREF(&CFI_desc_type);
    if (PyModule_AddObject(m, "CFI_desc_t", (PyObject *) &CFI_desc_type) < 0) {
        goto except;
    }

    if(add_constants(m)){
        goto except;
    }

    if(add_compiler_constants(m)){
        goto except;
    }

    if(set_compiler(m)){
        goto except;
    }

    if(add_types(m)){
        goto except;
    }

    if(add_compiler_types(m)){
        goto except;
    }   

    goto finally;

except:
    Py_XDECREF(&CFI_desc_type);
    Py_XDECREF(m);
    m = NULL;
finally:
    return m;
}


static int add_constants(PyObject *m){

    /* Adding module globals */
    if (PyModule_AddIntMacro(m, CFI_MAX_RANK)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_VERSION)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_attribute_pointer)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_attribute_allocatable)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_attribute_other)) {
        return 1;
    }

    if (PyModule_AddIntMacro(m, CFI_SUCCESS)) {
        return 1;
    }   
    if (PyModule_AddIntMacro(m, CFI_ERROR_BASE_ADDR_NULL)) {
        return 1;
    }   
    if (PyModule_AddIntMacro(m, CFI_ERROR_BASE_ADDR_NOT_NULL)) {
        return 1;
    }   
    if (PyModule_AddIntMacro(m, CFI_INVALID_RANK)) {
        return 1;
    }   
    if (PyModule_AddIntMacro(m, CFI_INVALID_TYPE)) {
        return 1;
    } 
    if (PyModule_AddIntMacro(m, CFI_INVALID_ATTRIBUTE)) {
        return 1;
    } 
    if (PyModule_AddIntMacro(m, CFI_INVALID_EXTENT)) {
        return 1;
    } 
    if (PyModule_AddIntMacro(m, CFI_INVALID_DESCRIPTOR)) {
        return 1;
    } 
    if (PyModule_AddIntMacro(m, CFI_ERROR_MEM_ALLOCATION)) {
        return 1;
    } 
    if (PyModule_AddIntMacro(m, CFI_ERROR_OUT_OF_BOUNDS)) {
        return 1;
    } 

    return 0;
}

static int add_compiler_constants(PyObject *m){

    // GCC specifics
    #ifdef __GNUC__ 
    if (PyModule_AddIntMacro(m, CFI_FAILURE)) {
        return 1;
    }   
    if (PyModule_AddIntMacro(m, CFI_INVALID_STRIDE)) {
        return 1;
    } 
    #endif

    return 0;

}


static int set_compiler(PyObject *m){

    #ifdef __GNUC__
    if (PyModule_AddStringConstant(m, "IFB_COMPILER", "GCC")) {
        return 1;
    }  
    if (PyModule_AddStringConstant(m, "IFB_COMPILER_VERSION", __VERSION__)) {
        return 1;
    }  
    #elif __INTEL_COMPILER
    if (PyModule_AddStringConstant(m, "IFB_COMPILER", "ICC")) {
        return 1;
    }   
    if (PyModule_AddStringConstant(m, "IFB_COMPILER_VERSION", __VERSION__)) {
        return 1;
    }     
    #else
    if (PyModule_AddStringConstant(m, "IFB_COMPILER", "UNKNOWN")) {
        return 1;
    }   
    if (PyModule_AddIntConstant(m, "IFB_COMPILER_VERSION", -1)) {
        return 1;
    }      
    #endif

    return 0;
}

static int add_types(PyObject *m){
    
    if (PyModule_AddIntMacro(m, CFI_type_signed_char)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_short)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_int)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_long)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_long_long)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_size_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_int8_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_int16_t)) {
        return 1;
    }
        
    if (PyModule_AddIntMacro(m, CFI_type_int32_t)) {
        return 1;
    }
        
    if (PyModule_AddIntMacro(m, CFI_type_int64_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_int_least8_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_int_least16_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_int_least32_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_int_least64_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_int_fast8_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_int_fast16_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_int_fast32_t)) {
        return 1;
    }
        
    if (PyModule_AddIntMacro(m, CFI_type_int_fast64_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_intmax_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_intptr_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_ptrdiff_t)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_float)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_double)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_long_double)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_float_Complex)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_double_Complex)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_long_double_Complex)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_Bool)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_char)) {
        return 1;
    }

    if (PyModule_AddIntMacro(m, CFI_type_cptr)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_struct)) {
        return 1;
    }
          
    if (PyModule_AddIntMacro(m, CFI_type_other)) {
        return 1;
    }

    return 0;
}

static int add_compiler_types(PyObject *m){

    #ifdef __GNUC__
    if (PyModule_AddIntMacro(m, CFI_type_mask)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_type_kind_shift)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_type_Integer)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_type_Logical)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_type_Real)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_type_Complex)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_type_Character)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_type_cfunptr)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_type_float128)) {
        return 1;
    }
    if (PyModule_AddIntMacro(m, CFI_type_float128_Complex)) {
        return 1;
    }


    #endif


    return 0;
}
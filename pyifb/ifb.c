// SPDX-License-Identifier: GPL-2.0+

#include "ifb.h"

// https://github.com/gcc-mirror/gcc/blob/master/libgfortran/ISO_Fortran_binding.h
// https://wg5-fortran.org/N1901-N1950/N1942.pdf


static PyMemberDef PyCFI_dim_members[] = {
    {"lower_bound",PyCFI_index_t, offsetof(CFI_dim_object,dim.lower_bound),0,
        PyDoc_STR("The value is equal to the value of the lower bound for the dimension being described")},
    {"extent",PyCFI_index_t,offsetof(CFI_dim_object,dim.extent),0,
        PyDoc_STR("The value is equal to the number of elements along the dimension being described, or the value -1 for the final dimension of an assumed-size array.")},
    {"sm",PyCFI_index_t,offsetof(CFI_dim_object,dim.sm),0,
        PyDoc_STR("The value is equal to the memory stride for a dimension. The value is the distance in bytes between the beginnings of successive elements along the dimension being described.")},
    {NULL},  /* Sentinel */
};

static initproc PyCFI_dim_init(CFI_dim_object *self, PyObject *args, PyObject *kwds){

    if(self != NULL){ 
        self->dim.lower_bound = 0;
        self->dim.extent = 0;
        self->dim.sm = 0;
    }
    return 0;
}


static PyType_Slot PyCFI_dim_slots[] = {
    { Py_tp_doc, PyDoc_STR("CFI_dim_t holds the dimensions of an object") },
    { Py_tp_members, &PyCFI_dim_members},
    { Py_tp_init, *PyCFI_dim_init},
    {0, NULL},
};

static PyType_Spec PyCFI_dim_spec = {
    .name = "ifb.CFI_dim_t",
    .basicsize = sizeof(CFI_dim_object),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots = PyCFI_dim_slots,
};




// static PyMemberDef CFI_cdesc_members[] = {
//     {"base_addr",Py_T_INT, offsetof(CFI_cdesc_object,dv.base_addr),0,
//         PyDoc_STR("")},
//     {"elem_len",Py_T_INT,offsetof(CFI_cdesc_object,dv.elem_len),0,
//         PyDoc_STR("If the object is scalar, the value is the storage size in bytes of the object; otherwise, the value is the storage size in bytes of an element of the object")},
//     {"rank",Py_T_INT,offsetof(CFI_cdesc_object,dv.rank),0,
//         PyDoc_STR("The value is equal to the number of dimensions of the Fortran object being described. If the object is a scalar, the value is zero.")},
//     {"type",Py_T_INT,offsetof(CFI_cdesc_object,dv.type),0,
//         PyDoc_STR("The value is equal to the specifier for the type of the object.")},
//      {"attribute",Py_T_INT,offsetof(CFI_cdesc_object,dv.attribute),0,
//         PyDoc_STR("The value is equal to the value of an attribute code that indicates whether the object described is allocatable, a data pointer, or a nonallocatable nonpointer data object.")},
//         {NULL}  /* Sentinel */
// };

// static PyTypeObject CFI_cdesc_type = {
//     PyVarObject_HEAD_INIT(NULL, 0)
//     .tp_name = "CFI_cdesc_t",
//     .tp_doc = PyDoc_STR("CFI_cdesc_t holds the C descriptor"),
//     .tp_basicsize = sizeof(CFI_cdesc_object),
//     .tp_itemsize = 0,
//     .tp_flags = Py_TPFLAGS_DEFAULT,
//     .tp_new = PyType_GenericNew,
//     .tp_members = CFI_cdesc_members,
// };



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
    #if REALLY_GCC
    if (PyModule_AddIntMacro(m, CFI_FAILURE)) {
        return 1;
    }   
    if (PyModule_AddIntMacro(m, CFI_INVALID_STRIDE)) {
        return 1;
    } 
    // if (PyModule_AddIntConstant(m,"sizeof_cfi_index_t", sizeof(CFI_index_t))) {
    //     return 1;
    // } 

    #endif

    return 0;

}


static int set_compiler(PyObject *m){

    #if REALLY_GCC
    if (PyModule_AddStringConstant(m, "IFB_COMPILER", "GCC")) {
        return 1;
    }  
    if (PyModule_AddStringConstant(m, "IFB_COMPILER_VERSION", __VERSION__)) {
        return 1;
    } 
    #elif REALLY_ICX
    if (PyModule_AddStringConstant(m, "IFB_COMPILER", "ICX")) {
        return 1;
    }   
    if (PyModule_AddStringConstant(m, "IFB_COMPILER_VERSION", __VERSION__)) {
        return 1;
    }          
    #else
    if (PyModule_AddStringConstant(m, "IFB_COMPILER", "UNKNOWN")) {
        return 1;
    }   
    if (PyModule_AddStringConstant(m, "IFB_COMPILER_VERSION", "-1")) {
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

    // GCC specifics
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

    return 0;
}


static int add_pytypes(PyObject *m){

    PyObject *CFI_dim_type = PyType_FromSpec(&PyCFI_dim_spec);
    if (CFI_dim_type == NULL) {
        return -1;
    }

    int res = PyModule_AddObjectRef(m, "CFI_dim_t", CFI_dim_type);
    Py_XDECREF(&CFI_dim_type);
    return res;
}

static int IFBModule_exec(PyObject *module){
    if(add_constants(module)){
        return 1;
    };
    if(add_compiler_constants(module)){
        return 1;
    };
    if(set_compiler(module)){
        return 1;
    };
    if(add_types(module)){
        return 1;
    };
    if(add_compiler_types(module)){
        return 1;
    };

    if(add_pytypes(module)){
        return 1;
    };
 

    return 0;
};

#ifdef Py_mod_exec
static PyModuleDef_Slot IFBModule_slots[] = {
    {Py_mod_exec, IFBModule_exec},
#ifdef Py_mod_multiple_interpreters
    {Py_mod_multiple_interpreters, Py_MOD_PER_INTERPRETER_GIL_SUPPORTED}
#endif
#ifdef Py_mod_gil
    {Py_mod_gil, Py_MOD_GIL_NOT_USED},
#endif
    {0, NULL}
};
#endif


PyModuleDef IFBModule = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "ifb",
    .m_doc = PyDoc_STR("Bindings for ISO_Fortran_binding.h"),
    .m_size = 0,
#ifdef Py_mod_exec
    .m_slots = IFBModule_slots,
#endif
};


PyMODINIT_FUNC
PyInit_ifb(void) {
#ifdef Py_mod_exec
    return PyModuleDef_Init(&IFBModule);
#else
    PyObject *module;
    module = PyModule_Create(&IFBModule);
    if (module == NULL) return NULL;
    if (IFBModule_exec(module) != 0) {
        Py_DECREF(module);
        return NULL;
    }
    return module;
#endif
}
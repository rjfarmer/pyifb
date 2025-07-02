// SPDX-License-Identifier: GPL-2.0+

#include "ifb.h"

// https://github.com/gcc-mirror/gcc/blob/master/libgfortran/ISO_Fortran_binding.h
// https://wg5-fortran.org/N1901-N1950/N1942.pdf


static PyMemberDef CFI_dim_members[] = {
    {"lower_bound",Py_T_INT, offsetof(CFI_dim_object,dim.lower_bound),0,
        PyDoc_STR("The value is equal to the value of the lower bound for the dimension being described")},
    {"extent",Py_T_INT,offsetof(CFI_dim_object,dim.extent),0,
        PyDoc_STR("The value is equal to the number of elements along the dimension being described, or the value -1 for the final dimension of an assumed-size array.")},
    {"sm",Py_T_INT,offsetof(CFI_dim_object,dim.sm),0,
        PyDoc_STR("The value is equal to the memory stride for a dimension. The value is the distance in bytes between the beginnings of successive elements along the dimension being described.")},
    {NULL}  /* Sentinel */
};

static PyTypeObject CFI_dim_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "CFI_dim_t",
    .tp_doc = PyDoc_STR("CFI_dim_t holds the dimensions of an object"),
    .tp_basicsize = sizeof(CFI_dim_object),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_members = CFI_dim_members,
    // .tp_setattro = CFI_dim_setattro,
    // .tp_getattro = CFI_dim_getattro,
};



static PyMemberDef CFI_cdesc_members[] = {
    {"base_addr",Py_T_INT, offsetof(CFI_cdesc_object,dv.base_addr),0,
        PyDoc_STR("")},
    {"elem_len",Py_T_INT,offsetof(CFI_cdesc_object,dv.elem_len),0,
        PyDoc_STR("If the object is scalar, the value is the storage size in bytes of the object; otherwise, the value is the storage size in bytes of an element of the object")},
    {"rank",Py_T_INT,offsetof(CFI_cdesc_object,dv.rank),0,
        PyDoc_STR("The value is equal to the number of dimensions of the Fortran object being described. If the object is a scalar, the value is zero.")},
    {"type",Py_T_INT,offsetof(CFI_cdesc_object,dv.type),0,
        PyDoc_STR("The value is equal to the specifier for the type of the object.")},
     {"attribute",Py_T_INT,offsetof(CFI_cdesc_object,dv.attribute),0,
        PyDoc_STR("The value is equal to the value of an attribute code that indicates whether the object described is allocatable, a data pointer, or a nonallocatable nonpointer data object.")},
        {NULL}  /* Sentinel */
};

static PyTypeObject CFI_cdesc_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "CFI_cdesc_t",
    .tp_doc = PyDoc_STR("CFI_cdesc_t holds the C descriptor"),
    .tp_basicsize = sizeof(CFI_cdesc_object),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_members = CFI_cdesc_members,
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

    if (PyType_Ready(&CFI_dim_type) < 0){
        goto except;
    }
        
    Py_INCREF(&CFI_dim_type);
    if (PyModule_AddObject(m, "CFI_dim_t", (PyObject *) &CFI_dim_type) < 0) {
        goto except;
    }

    if (PyType_Ready(&CFI_cdesc_type) < 0){
        goto except;
    }
        
    Py_INCREF(&CFI_cdesc_type);
    if (PyModule_AddObject(m, "CFI_cdesc_t", (PyObject *) &CFI_cdesc_type) < 0) {
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
    Py_XDECREF(&CFI_dim_type);
    Py_XDECREF(&CFI_cdesc_type);
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

static int check_return(int error_code){

    switch (error_code){
        case CFI_SUCCESS:
            // No error happened
            return 0;
        case CFI_ERROR_BASE_ADDR_NULL:
            PyErr_SetString(PyExc_ValueError, "The base address member of a C descriptor is a null pointer in a context that requires a non-null pointer value.");
            return 1;
        case CFI_ERROR_BASE_ADDR_NOT_NULL:
            PyErr_SetString(PyExc_ValueError, "The base address member of a C descriptor is not a null pointer in a context that requires a null pointer value.");
            return 1;
        case CFI_INVALID_RANK:
            PyErr_SetString(PyExc_ValueError, "The value supplied for the element length member of a C descriptor is not valid.");
            return 1;
        case CFI_INVALID_TYPE:
            PyErr_SetString(PyExc_ValueError, "The value supplied for the type member of a C descriptor is not valid.");
            return 1;
        case CFI_INVALID_ATTRIBUTE:
            PyErr_SetString(PyExc_ValueError, "The value supplied for the attribute member of a C descriptor is not valid.");
            return 1;
        case CFI_INVALID_EXTENT:
            PyErr_SetString(PyExc_ValueError, "The value supplied for the extent member of a CFI_dim_t structure is not valid.");
            return 1;
        case CFI_INVALID_DESCRIPTOR:
            PyErr_SetString(PyExc_ValueError, "A general error condition for C descriptors.");
            return 1;
        case CFI_ERROR_MEM_ALLOCATION:
            PyErr_SetString(PyExc_ValueError, "Memory allocation failed.");
            return 1;
        case CFI_ERROR_OUT_OF_BOUNDS:
            PyErr_SetString(PyExc_ValueError, "A reference is out of bounds.");
            return 1;
        #ifdef __GNUC__
        case CFI_FAILURE:
            PyErr_SetString(PyExc_ValueError, "An error occurred");
            return 1;
        case CFI_INVALID_STRIDE:
            PyErr_SetString(PyExc_ValueError, "Invalid stride");
            return 1;
        #endif      

    }

    PyErr_SetString(PyExc_ValueError, "Unknown error occurred.");

    return 1;
}

// CFI_dim methods and members


// static PyObject* CFI_dim_getattro(CFI_dim_object *self, PyObject *attr){

//     char *str = NULL;
//     PyObject *out = NULL;

//     str = PyUnicode_AsUTF8String(attr);
//     if(str == NULL){
//         return NULL;
//     }

//     Py_INCREF(out);
//     if(strcmp(str,"lower_bound")==0){
//         out = PyLong_FromLong(self->dim.lower_bound);
//     }
//     else if(strcmp(str,"extent")==0){
//         out = PyLong_FromLong(self->dim.extent);
//     }
//     else if(strcmp(str,"sm")==0){
//         out = PyLong_FromLong(self->dim.sm);
//     }


//     return out;

// }


// static int CFI_dim_setattro(CFI_dim_object *self, PyObject *attr, PyObject *value){
    
// }
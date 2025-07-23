// SPDX-License-Identifier: GPL-2.0+

#include "ifb.h"

// https://github.com/gcc-mirror/gcc/blob/master/libgfortran/ISO_Fortran_binding.h
// https://wg5-fortran.org/N1901-N1950/N1942.pdf


// START CFI_dimt_t setup

static PyMemberDef PyCFI_dim_members[] = {
    {"lower_bound",PyCFI_index_t, offsetof(PyCFI_dim_object,dim.lower_bound),Py_READONLY,
        PyDoc_STR("The value is equal to the value of the lower bound for the dimension being described")},
    {"extent",PyCFI_index_t,offsetof(PyCFI_dim_object,dim.extent),Py_READONLY,
        PyDoc_STR("The value is equal to the number of elements along the dimension being described, or the value -1 for the final dimension of an assumed-size array.")},
    {"sm",PyCFI_index_t,offsetof(PyCFI_dim_object,dim.sm),Py_READONLY,
        PyDoc_STR("The value is equal to the memory stride for a dimension. The value is the distance in bytes between the beginnings of successive elements along the dimension being described.")},
    {NULL},  /* Sentinel */
};

static PyType_Slot PyCFI_dim_slots[] = {
    { Py_tp_doc, PyDoc_STR("CFI_dim_t holds the dimensions of an object") },
    { Py_tp_members, &PyCFI_dim_members},
    {0, NULL},
};

static PyType_Spec PyCFI_dim_spec = {
    .name = "ifb.CFI_dim_t",
    .basicsize = sizeof(PyCFI_dim_object),
    .itemsize = 0,
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots = PyCFI_dim_slots,
};

static PyObject* new_PyCFI_dim(){

    PyCFI_dim_object *out;

    PyObject *CFI_dim_type = PyType_FromSpec(&PyCFI_dim_spec);
    if (CFI_dim_type == NULL) {
        Py_RETURN_NONE;
    }

    out = (PyCFI_dim_object*) PyObject_CallObject(CFI_dim_type, NULL);
    Py_XDECREF(CFI_dim_type);

    return (PyObject*) out;

}


static PyObject* PyCFI_dim_object_from_CFI_dim_t(CFI_dim_t in){

    PyCFI_dim_object *out = (PyCFI_dim_object*) new_PyCFI_dim();

    if(out){out->dim = in;}

    return (PyObject*) out; 
}


// END CFI_dimt_t setup
/////////////////////////////////////////////////////////


// START CFI_cdesc setup


static PyMemberDef PyCFI_cdesc_members[] = {
    {"elem_len",Py_T_PYSSIZET,offsetof(PyCFI_cdesc_object,dv.elem_len),Py_READONLY,
        PyDoc_STR("If the object is scalar, the value is the storage size in bytes of the object; otherwise, the value is the storage size in bytes of an element of the object")},
    {"type",PyCFI_type_t,offsetof(PyCFI_cdesc_object,dv.type),Py_READONLY,
        PyDoc_STR("The value is equal to the specifier for the type of the object.")},
    {"version",Py_T_INT,offsetof(PyCFI_cdesc_object,dv.version),Py_READONLY,
        PyDoc_STR("The value is equal to the value of CFI VERSION in the ISO_Fortran_binding.h header file that defined the format and meaning of this C descriptor when the descriptor was established.")},
        {NULL}  /* Sentinel */
};


static PyObject* PyCFI_cdesc_base_addr_get(PyCFI_cdesc_object* self, void* Py_UNUSED){
    return PyLong_FromVoidPtr(self->dv.base_addr);
}

static PyObject* PyCFI_cdesc_rank_get(PyCFI_cdesc_object* self, void* Py_UNUSED){
    return PyLong_FromLong((long) self->dv.rank);
}

static PyObject* PyCFI_cdesc_attribute_get(PyCFI_cdesc_object* self, void* Py_UNUSED){
    return PyLong_FromLong((long) self->dv.attribute);
}

static PyObject* PyCFI_cdesc_dim_get(PyCFI_cdesc_object* self, void* Py_UNUSED){

    int8_t rank = self->dv.rank;
    PyObject *tmp;

    if(rank==0){
        Py_RETURN_NONE;
    }

    PyObject *dims = PyTuple_New((Py_ssize_t) rank);
    if(dims == NULL) {
        Py_RETURN_NONE;
    }

    int res;
    for(int i = 0; i < (int) rank; i++){
        tmp = PyCFI_dim_object_from_CFI_dim_t(self->dv.dim[i]);
        Py_INCREF(tmp);
        res = PyTuple_SetItem(dims, i, tmp);
        if(res!=0) {
            Py_XDECREF(dims);
            PyErr_Print();
            PyErr_SetString(PyExc_ValueError, "Error setting dimension");
            Py_RETURN_NONE;
        }
    }

    return dims;
}


static void PyCFI_cdesc_dealloc(PyCFI_cdesc_object *self) {
    PyTypeObject *tp = Py_TYPE(self);
    CFI_deallocate(&self->dv);
    ((freefunc)PyType_GetSlot(Py_TYPE(self), Py_tp_free))(self);
    Py_DECREF(tp);
}

static newfunc PyCFI_cdesc_new(PyTypeObject *subtype, PyObject *args, void* Py_UNUSED){

    int rank=0;
    PyCFI_cdesc_object *self;

    if (!PyArg_ParseTuple(args, "|i:", &rank)){
        if(PyErr_Occurred()){
            return NULL;
        }
    }

    if(rank>CFI_MAX_RANK){
        PyErr_SetString(PyExc_ValueError, "Rank must be less than " STR_CFI_MAX_RANK "");
        return NULL;
    }

    if(rank<0){
        PyErr_SetString(PyExc_ValueError, "Rank must be greater than zero");
        return NULL;
    }

    printf("Rank of %d %d\n",rank,CFI_MAX_RANK);

    self = (PyCFI_cdesc_object*) ((allocfunc)PyType_GetSlot(Py_TYPE(subtype), Py_tp_alloc))(subtype, (Py_ssize_t) rank);
        
    self->dv.rank = (CFI_rank_t) rank;
    self->dv.base_addr = NULL;

    return (newfunc) self;

}


static PyObject* PyCFI_cdesc_from_bytes(PyTypeObject *type, PyObject * arg){

    if(!PyBytes_Check(arg)){
        PyErr_SetString(PyExc_TypeError, "Must be a bytes object");
        return NULL;
    }

    char* bytes = PyBytes_AsString(arg);
    int rank;

    memcpy(&rank, &bytes[offsetof(CFI_cdesc_t,rank)], sizeof(CFI_rank_t));

    printf("Got rank %d\n",rank);
    PyCFI_cdesc_object* self = (PyCFI_cdesc_object*) new_PyCFI_cdesc(PyLong_FromLong(rank));

    memcpy(&self->dv, bytes, PyBytes_Size(arg));

    return (PyObject *) self;
}

static PyObject* PyCFI_cdesc_to_bytes(PyCFI_cdesc_object *self, PyObject * args){

    Py_ssize_t size = sizeof(CFI_cdesc_t) + sizeof(CFI_dim_t) * self->dv.rank;

    unsigned char buffer[size];

    memcpy(buffer, &self->dv, size);

    return PyBytes_FromStringAndSize(buffer, size);

}

static PyObject* PyCFI_cdesc_as_parameter_(PyCFI_cdesc_object * self, void * y){
    return PyCFI_cdesc_to_bytes(self, NULL);
}



static PyMethodDef PyCFI_cdesc_methods[] = {
    {"to_bytes", (PyCFunction) PyCFI_cdesc_to_bytes, METH_NOARGS, "to_bytes"},
    {"from_bytes", (PyCFunction) PyCFI_cdesc_from_bytes, METH_CLASS|METH_O, "from_bytes"},
    {"from_param", (PyCFunction) PyCFI_cdesc_to_bytes, METH_CLASS|METH_O, "from_param"},
    {NULL}
};


static PyGetSetDef PyCFI_cdesc_getset[] = {
    {
        .name = "base_addr",
        .get = (getter) PyCFI_cdesc_base_addr_get,
        .doc = PyDoc_STR("The value is the base address of the object being described.")
    },
    {
        .name = "rank",
        .get = (getter) PyCFI_cdesc_rank_get,
        .doc = PyDoc_STR("The value is equal to the number of dimensions of the Fortran object being described. If the object is a scalar, the value is zero.")
    },
    {
        .name = "attribute",
        .get = (getter) PyCFI_cdesc_attribute_get,
        .doc = PyDoc_STR("The value is equal to the value of an attribute code that indicates whether the object described is allocatable, a data pointer, or a nonallocatable nonpointer data object.")
    },
    {
        .name = "dim",
        .get = (getter) PyCFI_cdesc_dim_get,
        .doc = PyDoc_STR("The number of elements in the dim array is equal to the rank of the object. Each element of the array contains the lower bound, extent, and memory stride information for the corresponding dimension of the Fortran object.")
    },
    {
        .name = "_as_parameter",
        .get = (getter) PyCFI_cdesc_as_parameter_,
        .doc = PyDoc_STR("ctypes _as_parameter")
    },

    {NULL}
};


static PyType_Slot PyCFI_cdesc_slots[] = {
    { Py_tp_doc, PyDoc_STR("CFI_cdesc_t holds the C descriptor. Initialize with rank (up to CFI_MAX_RANK) or defaults to rank-0")},
    { Py_tp_members, &PyCFI_cdesc_members},
    { Py_tp_methods, &PyCFI_cdesc_methods},
    { Py_tp_getset, &PyCFI_cdesc_getset},
    { Py_tp_dealloc, PyCFI_cdesc_dealloc},
    { Py_tp_new, PyCFI_cdesc_new},
    {0, NULL},
};

static PyType_Spec PyCFI_cdesc_spec = {
    .name = "ifb.CFI_cdesc_t",
    .basicsize = sizeof(PyCFI_cdesc_object),
    .itemsize = sizeof(CFI_dim_t),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots = PyCFI_cdesc_slots,
};


static PyObject* new_PyCFI_cdesc(PyObject* rank){

    PyCFI_cdesc_object *out;

    PyObject *PyCFI_cdesc_type = PyType_FromSpec(&PyCFI_cdesc_spec);
    if (PyCFI_cdesc_type == NULL) {
        Py_RETURN_NONE;
    }

    PyObject *dims = PyTuple_New((Py_ssize_t) 1);
    if(dims == NULL) {
        Py_RETURN_NONE;
    }

    Py_INCREF(rank);
    int res = PyTuple_SetItem(dims, 0, rank);
    if(res!=0) {
        Py_XDECREF(dims);
        Py_XDECREF(rank);
        Py_XDECREF(PyCFI_cdesc_type);
        PyErr_SetString(PyExc_ValueError, "Error setting up new dimension\n");
        Py_RETURN_NONE;
    }

    out = (PyCFI_cdesc_object*) PyObject_CallObject(PyCFI_cdesc_type, dims);
    Py_XDECREF(PyCFI_cdesc_type);

    return (PyObject*) out;

}




// END CFI_desc setup
/////////////////////////////////////////////////////////



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

    int res;

    PyObject *PyCFI_dim_type = PyType_FromSpec(&PyCFI_dim_spec);
    if (PyCFI_dim_type == NULL) {
        return -1;
    }

    res = PyModule_AddObjectRef(m, "CFI_dim_t", PyCFI_dim_type);
    Py_XDECREF(&PyCFI_dim_type);
    if (res == -1 ) {
        return res;
    }


    PyObject *PyCFI_cdesc_type = PyType_FromSpec(&PyCFI_cdesc_spec);
    if (PyCFI_cdesc_type == NULL) {
        return -1;
    }

    res = PyModule_AddObjectRef(m, "CFI_cdesc_t", PyCFI_cdesc_type);
    Py_XDECREF(&PyCFI_cdesc_type);
    if (res == -1 ) {
        return res;
    }

    return 0;
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
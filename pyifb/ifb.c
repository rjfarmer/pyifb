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
        return NULL;
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
    if(self->dv.base_addr==NULL) {
        return Py_None;
    }
    return PyLong_FromVoidPtr(self->dv.base_addr);
}

static PyObject* PyCFI_cdesc_rank_get(PyCFI_cdesc_object* self, void* Py_UNUSED){
    return PyLong_FromLong((long) self->dv.rank);
}

static PyObject* PyCFI_cdesc_attribute_get(PyCFI_cdesc_object* self, void* Py_UNUSED){
    return PyLong_FromLong((long) self->dv.attribute);
}

static PyObject* PyCFI_cdesc_dim_get(PyCFI_cdesc_object* self, void* Py_UNUSED){

    CFI_rank_t rank = self->dv.rank;

    if(rank==0){
        Py_RETURN_NONE;
    }

    PyObject *dims = PyTuple_New((Py_ssize_t) rank);
    if(dims == NULL) {
        return NULL;
    }

    PyObject *dim_obj;
    int set_result;
    for(int i = 0; i < rank; i++){
        dim_obj = PyCFI_dim_object_from_CFI_dim_t(self->dv.dim[i]);
        if (dim_obj == NULL) {
            Py_XDECREF(dims);
            return NULL;
        }
        set_result = PyTuple_SetItem(dims, i, dim_obj);
        if(set_result != 0) {
            Py_XDECREF(dims);
            PyErr_SetString(PyExc_ValueError, "Error setting dimension");
            return NULL;
        }
    }

    return dims;
}


static newfunc PyCFI_cdesc_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds){

    CFI_rank_t rank=0;
    int rank_int=0;

    if (!PyArg_ParseTuple(args, "|i", &rank_int)){
        if(PyErr_Occurred()){
            return NULL;
        }
        rank_int = 0;
    }
    rank = (CFI_rank_t) rank_int;

    if(rank>CFI_MAX_RANK){
        PyErr_SetString(PyExc_ValueError, "Rank must be less than " STR_CFI_MAX_RANK "");
        return NULL;
    }

    if(rank<0){
        PyErr_SetString(PyExc_ValueError, "Rank must be greater than zero");
        return NULL;
    }

    PyCFI_cdesc_object *self;
    self = (PyCFI_cdesc_object*) ((allocfunc)PyType_GetSlot(Py_TYPE(subtype), Py_tp_alloc))(subtype, (Py_ssize_t) rank);
    if (self == NULL) {
        return NULL;
    }

    memset(&self->dv, 0, sizeof(CFI_cdesc_t) + rank * sizeof(CFI_dim_t));
    self->dv.rank = rank;
    self->dv.base_addr = NULL;

    return (newfunc) self;

}


static PyObject* PyCFI_cdesc_from_bytes(PyTypeObject *type, PyObject * arg){

    if(!PyBytes_Check(arg)){
        PyErr_SetString(PyExc_TypeError, "Must be a bytes object");
        return NULL;
    }

    // Validate bytes size to prevent buffer overflow and ensure data integrity
    Py_ssize_t bytes_size = PyBytes_Size(arg);
    if (bytes_size < (Py_ssize_t)sizeof(CFI_cdesc_t)) {
        PyErr_SetString(PyExc_ValueError, "Bytes object too small");
        return NULL;
    }

    char* bytes = PyBytes_AsString(arg);
    CFI_rank_t rank;

    // Extract rank from bytes and calculate expected size for validation
    memcpy(&rank, &bytes[offsetof(CFI_cdesc_t,rank)], sizeof(CFI_rank_t));

    size_t expected_size = sizeof(CFI_cdesc_t) + sizeof(CFI_dim_t) * rank;
    if (bytes_size != (Py_ssize_t)expected_size) {
        PyErr_SetString(PyExc_ValueError, "Invalid bytes size");
        return NULL;
    }

    PyObject* obj_rank = PyLong_FromLong(rank);
    if (obj_rank == NULL) {
        return NULL;
    }

    PyObject *dims = PyTuple_New(1);
    if (dims == NULL) {
        Py_DECREF(obj_rank);
        return NULL;
    }
    if (PyTuple_SetItem(dims, 0, obj_rank) != 0) {  /* SetItem steals obj_rank ref */
        Py_DECREF(dims);
        return NULL;
    }   /* obj_rank consumed */

    PyCFI_cdesc_object* self = (PyCFI_cdesc_object*) PyObject_CallObject((PyObject *)type, dims);
    Py_DECREF(dims);
    if (self == NULL) {
        return NULL;
    }

    memcpy(&self->dv, bytes, bytes_size);

    return (PyObject *) self;
}

static PyObject* PyCFI_cdesc_to_bytes(PyCFI_cdesc_object *self, PyObject * Py_UNUSED){

    Py_ssize_t size = sizeof(CFI_cdesc_t) + sizeof(CFI_dim_t) * self->dv.rank;

    char *buffer = (char *)malloc((size_t)size);
    if (buffer == NULL) {
        return PyErr_NoMemory();
    }

    memcpy(buffer, &self->dv, size);
    PyObject *result = PyBytes_FromStringAndSize(buffer, size);
    free(buffer);
    return result;

}

static PyObject* PyCFI_cdesc_as_parameter_(PyCFI_cdesc_object * self, PyObject * Py_UNUSED){
    return PyCFI_cdesc_to_bytes(self, NULL);
}

static PyObject* PyCFI_cdesc_allocate(PyCFI_cdesc_object *self, PyObject *args) {
    /* Allocate memory for the descriptor array.
       Args: lower_bounds (sequence), upper_bounds (sequence), elem_len (int)
       Returns: Status code (CFI_SUCCESS or error code)
    */
    PyObject *lower_bounds_seq, *upper_bounds_seq;
    size_t elem_len;
    CFI_rank_t rank;
    CFI_index_t *lower_bounds, *upper_bounds;
    int status;

    if (!PyArg_ParseTuple(args, "OOK", &lower_bounds_seq, &upper_bounds_seq, &elem_len)) {
        return NULL;
    }

    rank = self->dv.rank;

    /* Validate descriptor state */
    if (self->dv.base_addr != NULL) {
        PyErr_SetString(PyExc_ValueError, "Descriptor base_addr must be NULL before allocate");
        return NULL;
    }

    /* Save mutable fields in case CFI_allocate fails and we need to restore them */
    int saved_version = self->dv.version;
    CFI_type_t saved_type = self->dv.type;
    CFI_attribute_t saved_attribute = self->dv.attribute;

    if (self->dv.version == 0) {
        self->dv.version = CFI_VERSION;
    }
    if (self->dv.type == 0) {
        self->dv.type = CFI_type_other;
    }

    if (self->dv.attribute != CFI_attribute_allocatable && self->dv.attribute != CFI_attribute_pointer) {
        // If not set, default to allocatable
        self->dv.attribute = CFI_attribute_allocatable;
    }

    /* Validate elem_len based on type */
    int is_character_type = (self->dv.type == CFI_type_char);
    if (is_character_type) {
        if (elem_len < 1) {
            PyErr_SetString(PyExc_ValueError, "For character types, elem_len must be at least 1");
            return NULL;
        }
    } else {
        // For non-character, elem_len should be positive
        if (elem_len == 0) {
            PyErr_SetString(PyExc_ValueError, "elem_len must be positive");
            return NULL;
        }
    }

    /* Set elem_len in descriptor */
    self->dv.elem_len = elem_len;

    /* Validate sequence lengths match rank */
    Py_ssize_t lower_len = PySequence_Length(lower_bounds_seq);
    Py_ssize_t upper_len = PySequence_Length(upper_bounds_seq);

    if (lower_len == -1 || upper_len == -1) {
        return NULL;  /* Error already set by PySequence_Length */
    }

    if (lower_len < (Py_ssize_t)rank || upper_len < (Py_ssize_t)rank) {
        PyErr_SetString(PyExc_ValueError, "Bounds sequences must have at least rank elements");
        return NULL;
    }

    /* Allocate temporary arrays for C function */
    lower_bounds = (CFI_index_t *)malloc(rank * sizeof(CFI_index_t));
    upper_bounds = (CFI_index_t *)malloc(rank * sizeof(CFI_index_t));

    if (lower_bounds == NULL || upper_bounds == NULL) {
        free(lower_bounds);
        free(upper_bounds);
        return PyErr_NoMemory();
    }

    /* Convert Python sequences to C arrays */
    for (CFI_rank_t i = 0; i < rank; i++) {
        PyObject *lb_item = PySequence_GetItem(lower_bounds_seq, i);
        PyObject *ub_item = PySequence_GetItem(upper_bounds_seq, i);

        if (lb_item == NULL || ub_item == NULL) {
            free(lower_bounds);
            free(upper_bounds);
            Py_XDECREF(lb_item);
            Py_XDECREF(ub_item);
            return NULL;
        }

        lower_bounds[i] = PyLong_AsLong(lb_item);
        upper_bounds[i] = PyLong_AsLong(ub_item);

        Py_DECREF(lb_item);
        Py_DECREF(ub_item);

        if (PyErr_Occurred()) {
            free(lower_bounds);
            free(upper_bounds);
            return NULL;
        }
    }

    /* Call CFI_allocate */
    status = CFI_allocate(&self->dv, lower_bounds, upper_bounds, elem_len);

    /* Clean up temporary arrays */
    free(lower_bounds);
    free(upper_bounds);

    if (status != CFI_SUCCESS) {
        /* Restore mutated fields so the descriptor is unchanged on failure */
        self->dv.elem_len = 0;
        self->dv.version = saved_version;
        self->dv.type = saved_type;
        self->dv.attribute = saved_attribute;
    }

    return PyLong_FromLong((long)status);
}

static void _PyCFI_cdesc_free_fortran(PyCFI_cdesc_object *self) {
    /* Free the Fortran array memory if this descriptor owns it (allocatable only). */
    if (self->dv.base_addr != NULL && self->dv.attribute == CFI_attribute_allocatable) {
        CFI_deallocate(&self->dv);
        self->dv.base_addr = NULL;
    }
}

static void PyCFI_cdesc_dealloc(PyCFI_cdesc_object *self) {
    PyTypeObject *tp = Py_TYPE(self);
    _PyCFI_cdesc_free_fortran(self);
    PyObject_Free(self);
    Py_DECREF(tp);
}

static PyObject* PyCFI_cdesc_deallocate(PyCFI_cdesc_object *self) {
    /* Deallocate the Fortran array memory; keeps the Python descriptor alive.
       Returns: CFI_SUCCESS (0) always
    */
    _PyCFI_cdesc_free_fortran(self);
    return PyLong_FromLong((long)CFI_SUCCESS);
}

static PyObject* PyCFI_cdesc_establish(PyCFI_cdesc_object *self, PyObject *args) {
    /* Establish a descriptor with given properties.
       Args: base_addr (void*), attribute (int), type (int), elem_len (int), rank (int), extents (sequence)
       Returns: Status code (CFI_SUCCESS or error code)
    */
    PyObject *base_addr_obj, *extents_seq, *attribute_obj, *type_obj, *elem_len_obj, *rank_obj;
    CFI_attribute_t attribute;
    CFI_type_t type;
    size_t elem_len;
    CFI_rank_t rank;
    CFI_index_t *extents;
    void *base_addr;
    int status;


    if (!PyArg_ParseTuple(args, "OOOOOO", &base_addr_obj, &attribute_obj, &type_obj, &elem_len_obj, &rank_obj, &extents_seq)) {
        return NULL;
    }

    /* Convert base_addr from Python object */
    if (Py_IsNone(base_addr_obj)) {
       base_addr = NULL;
    } else if (PyLong_Check(base_addr_obj)) {
       base_addr = PyLong_AsVoidPtr(base_addr_obj);
    } else {
        PyErr_SetString(PyExc_TypeError, "base_addr must be None or an integer");
        return NULL;
    }

    if (PyLong_Check(attribute_obj)) {
        attribute = (CFI_attribute_t) PyLong_AsLong(attribute_obj);
    } else {
        PyErr_SetString(PyExc_TypeError, "attribute must be an integer");
        return NULL;
    }

    if(PyLong_Check(type_obj)) {
        type = (CFI_type_t) PyLong_AsLong(type_obj);
    } else {
        PyErr_SetString(PyExc_TypeError, "type must be an integer");
        return NULL;
    }

    if(PyLong_Check(elem_len_obj)) {
        elem_len = (size_t) PyLong_AsSize_t(elem_len_obj);
    } else {
        PyErr_SetString(PyExc_TypeError, "elem_len must be an integer");
        return NULL;
    }

    if(PyLong_Check(rank_obj)) {
        rank = (CFI_rank_t) PyLong_AsLong(rank_obj);
    } else {
        PyErr_SetString(PyExc_TypeError, "rank must be an integer");
        return NULL;
    }

    /* Validate extents sequence length */
    Py_ssize_t extents_len = PySequence_Length(extents_seq);
    if (extents_len == -1) {
        return NULL;
    }
    if (extents_len != (Py_ssize_t)rank) {
        PyErr_SetString(PyExc_ValueError, "extents sequence must match rank");
        return NULL;
    }

    /* Allocate and convert extents */
    extents = (CFI_index_t *)malloc(rank * sizeof(CFI_index_t));
    if (extents == NULL) {
        return PyErr_NoMemory();
    }

    for (CFI_rank_t i = 0; i < rank; i++) {
        PyObject *ext_item = PySequence_GetItem(extents_seq, i);
        if (ext_item == NULL) {
            free(extents);
            return NULL;
        }
        extents[i] = PyLong_AsLong(ext_item);
        Py_DECREF(ext_item);
        if (PyErr_Occurred()) {
            free(extents);
            return NULL;
        }
    }

    /* Call CFI_establish */
    status = CFI_establish(&self->dv, base_addr, attribute, type, elem_len, rank, extents);

    /* Clean up */
    free(extents);

    return PyLong_FromLong((long)status);
}

static PyObject* PyCFI_cdesc_is_contiguous(PyCFI_cdesc_object *self) {
    /* Check if the array is contiguous in memory.
       Returns: 1 if contiguous, 0 if not, -1 on error
    */
    int result = CFI_is_contiguous(&self->dv);
    return result == 1 ? Py_True : Py_False;
}

static PyObject* PyCFI_cdesc_section(PyCFI_cdesc_object *self, PyObject *args) {
    /* Create a section/subarray of this descriptor.
       Args: result_desc (CFI_cdesc_t), lower_bounds (sequence or None), upper_bounds (sequence or None), strides (sequence or None)
       Returns: Status code (CFI_SUCCESS or error code)
    */
    PyCFI_cdesc_object *result_desc;
    PyObject *lower_bounds_seq = NULL, *upper_bounds_seq = NULL, *strides_seq = NULL, *result_desc_obj;
    CFI_rank_t rank, result_rank;
    CFI_index_t *lower_bounds = NULL, *upper_bounds = NULL, *strides = NULL;
    int status;

    if (!PyArg_ParseTuple(args, "O|OOO", &result_desc_obj, &lower_bounds_seq, &upper_bounds_seq, &strides_seq)) {
        return NULL;
    }

    if (!PyObject_TypeCheck(result_desc_obj, Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "result_desc must be a CFI_cdesc_t");
        return NULL;
    }

    result_desc = (PyCFI_cdesc_object *)result_desc_obj;

    /* Initialize result descriptor with properties from source */
    result_desc->dv.attribute = CFI_attribute_other;
    result_desc->dv.elem_len = self->dv.elem_len;
    result_desc->dv.type = self->dv.type;
    result_desc->dv.version = self->dv.version;

    rank = self->dv.rank;

    /* Validate sequence lengths if provided */
    Py_ssize_t lower_len = -1, upper_len = -1, strides_len = -1;
    
    if (lower_bounds_seq != NULL && lower_bounds_seq != Py_None) {
        lower_len = PySequence_Length(lower_bounds_seq);
        if (lower_len == -1) return NULL;
        if (lower_len < (Py_ssize_t)rank) {
            PyErr_SetString(PyExc_ValueError, "lower_bounds sequence must have at least rank elements");
            return NULL;
        }
    }
    
    if (upper_bounds_seq != NULL && upper_bounds_seq != Py_None) {
        upper_len = PySequence_Length(upper_bounds_seq);
        if (upper_len == -1) return NULL;
        if (upper_len < (Py_ssize_t)rank) {
            PyErr_SetString(PyExc_ValueError, "upper_bounds sequence must have at least rank elements");
            return NULL;
        }
    }
    
    if (strides_seq != NULL && strides_seq != Py_None) {
        strides_len = PySequence_Length(strides_seq);
        if (strides_len == -1) return NULL;
        if (strides_len < (Py_ssize_t)rank) {
            PyErr_SetString(PyExc_ValueError, "strides sequence must have at least rank elements");
            return NULL;
        }
    }

    /* Check for assumed-size array when upper_bounds is NULL */
    if ((upper_bounds_seq == NULL || upper_bounds_seq == Py_None) && rank > 0) {
        /* Check if any dimension has extent -1 (assumed-size) */
        int has_assumed_size = 0;
        for (CFI_rank_t i = 0; i < rank; i++) {
            if (self->dv.dim[i].extent == -1) {
                has_assumed_size = 1;
                break;
            }
        }
        if (has_assumed_size) {
            PyErr_SetString(PyExc_ValueError, "upper_bounds cannot be None for assumed-size arrays");
            return NULL;
        }
    }

    /* Allocate and convert arrays if provided */
    if (lower_bounds_seq != NULL && lower_bounds_seq != Py_None) {
        lower_bounds = (CFI_index_t *)malloc(rank * sizeof(CFI_index_t));
        if (lower_bounds == NULL) {
            return PyErr_NoMemory();
        }
        for (CFI_rank_t i = 0; i < rank; i++) {
            PyObject *lb_item = PySequence_GetItem(lower_bounds_seq, i);
            if (lb_item == NULL) {
                free(lower_bounds);
                return NULL;
            }
            lower_bounds[i] = PyLong_AsLong(lb_item);
            Py_DECREF(lb_item);
            if (PyErr_Occurred()) {
                free(lower_bounds);
                return NULL;
            }
        }
    }

    if (upper_bounds_seq != NULL && upper_bounds_seq != Py_None) {
        upper_bounds = (CFI_index_t *)malloc(rank * sizeof(CFI_index_t));
        if (upper_bounds == NULL) {
            free(lower_bounds);
            return PyErr_NoMemory();
        }
        for (CFI_rank_t i = 0; i < rank; i++) {
            PyObject *ub_item = PySequence_GetItem(upper_bounds_seq, i);
            if (ub_item == NULL) {
                free(lower_bounds);
                free(upper_bounds);
                return NULL;
            }
            upper_bounds[i] = PyLong_AsLong(ub_item);
            Py_DECREF(ub_item);
            if (PyErr_Occurred()) {
                free(lower_bounds);
                free(upper_bounds);
                return NULL;
            }
        }
    }

    if (strides_seq != NULL && strides_seq != Py_None) {
        strides = (CFI_index_t *)malloc(rank * sizeof(CFI_index_t));
        if (strides == NULL) {
            free(lower_bounds);
            free(upper_bounds);
            return PyErr_NoMemory();
        }
        for (CFI_rank_t i = 0; i < rank; i++) {
            PyObject *str_item = PySequence_GetItem(strides_seq, i);
            if (str_item == NULL) {
                free(lower_bounds);
                free(upper_bounds);
                free(strides);
                return NULL;
            }
            strides[i] = PyLong_AsLong(str_item);
            Py_DECREF(str_item);
            if (PyErr_Occurred()) {
                free(lower_bounds);
                free(upper_bounds);
                free(strides);
                return NULL;
            }
        }
    }

    /* Count zero strides to determine result rank */
    result_rank = rank;
    if (strides != NULL) {
        for (CFI_rank_t i = 0; i < rank; i++) {
            if (strides[i] == 0) {
                result_rank--;
            }
        }
    }

    /* Validate result descriptor rank */
    if (result_desc->dv.rank != result_rank) {
        free(lower_bounds);
        free(upper_bounds);
        free(strides);
        PyErr_SetString(PyExc_ValueError, "Result descriptor rank must equal source rank minus number of zero strides");
        return NULL;
    }

    /* Validate result descriptor attribute */
    if (result_desc->dv.attribute != CFI_attribute_other && result_desc->dv.attribute != CFI_attribute_pointer) {
        free(lower_bounds);
        free(upper_bounds);
        free(strides);
        PyErr_SetString(PyExc_ValueError, "Result descriptor attribute must be CFI_attribute_other or CFI_attribute_pointer");
        return NULL;
    }

    /* Validate result descriptor elem_len and type match source */
    if (result_desc->dv.elem_len != self->dv.elem_len) {
        free(lower_bounds);
        free(upper_bounds);
        free(strides);
        PyErr_SetString(PyExc_ValueError, "Result descriptor elem_len must match source descriptor elem_len");
        return NULL;
    }

    if (result_desc->dv.type != self->dv.type) {
        free(lower_bounds);
        free(upper_bounds);
        free(strides);
        PyErr_SetString(PyExc_ValueError, "Result descriptor type must match source descriptor type");
        return NULL;
    }

    /* Call CFI_section */
    status = CFI_section(&result_desc->dv, &self->dv, lower_bounds, upper_bounds, strides);

    /* Clean up */
    free(lower_bounds);
    free(upper_bounds);
    free(strides);

    return PyLong_FromLong((long)status);
}

static PyObject* PyCFI_cdesc_select_part(PyCFI_cdesc_object *self, PyObject *args) {
    /* Select a part of a descriptor (for derived types).
       Args: result_desc (CFI_cdesc_t), displacement (size_t), elem_len (size_t)
       Returns: Status code (CFI_SUCCESS or error code)
    */
    PyCFI_cdesc_object *result_desc;
    size_t displacement, elem_len;
    int status;

    if (!PyArg_ParseTuple(args, "OKK", &result_desc, &displacement, &elem_len)) {
        return NULL;
    }

    /* Validate source descriptor (self) */
    if (self->dv.base_addr == NULL) {
        PyErr_SetString(PyExc_ValueError, "Source descriptor base_addr must not be NULL");
        return NULL;
    }
    if (self->dv.attribute != CFI_attribute_other &&
        self->dv.attribute != CFI_attribute_allocatable &&
        self->dv.attribute != CFI_attribute_pointer) {
        PyErr_SetString(PyExc_ValueError, "Source descriptor attribute must be other, allocatable, or pointer");
        return NULL;
    }

    /* Validate displacement */
    if (displacement >= self->dv.elem_len) {
        PyErr_SetString(PyExc_ValueError, "Displacement must be less than source elem_len");
        return NULL;
    }

    /* Validate result descriptor */
    if (!PyObject_TypeCheck(result_desc, Py_TYPE(self))) {
        PyErr_SetString(PyExc_TypeError, "result_desc must be a CFI_cdesc_t");
        return NULL;
    }

    PyCFI_cdesc_object *res = (PyCFI_cdesc_object *)result_desc;

    /* Validate result descriptor rank matches source rank */
    if (res->dv.rank != self->dv.rank) {
        PyErr_SetString(PyExc_ValueError, "Result descriptor rank must match source descriptor rank");
        return NULL;
    }

    /* Validate result descriptor attribute */
    if (res->dv.attribute != CFI_attribute_other && res->dv.attribute != CFI_attribute_pointer) {
        PyErr_SetString(PyExc_ValueError, "Result descriptor attribute must be CFI_attribute_other or CFI_attribute_pointer");
        return NULL;
    }

    /* Validate result descriptor elem_len */
    int is_character_type = (res->dv.type == CFI_type_char);
    if (is_character_type) {
        if (elem_len < 1 || elem_len > self->dv.elem_len) {
            PyErr_SetString(PyExc_ValueError, "For character types, elem_len must be between 1 and source elem_len");
            return NULL;
        }
    } else {
        // For non-character types, elem_len should match the parameter
        if (res->dv.elem_len != elem_len) {
            PyErr_SetString(PyExc_ValueError, "Result elem_len must match provided elem_len");
            return NULL;
        }
    }

    status = CFI_select_part(&res->dv, &self->dv, displacement, elem_len);

    return PyLong_FromLong((long)status);
}

static PyObject* PyCFI_cdesc_setpointer(PyCFI_cdesc_object *self, PyObject *args) {
    /* Set this descriptor to point to another descriptor.
       Args: source_desc (CFI_cdesc_t or None), lower_bounds (sequence, optional)
       Returns: Status code (CFI_SUCCESS or error code)
    */
    PyObject *source_desc_obj;
    PyObject *lower_bounds_seq = NULL;
    PyCFI_cdesc_object *source_desc = NULL;
    CFI_rank_t source_rank = 0;
    CFI_index_t *lower_bounds = NULL;
    int status;

    if (!PyArg_ParseTuple(args, "O|O", &source_desc_obj, &lower_bounds_seq)) {
        return NULL;
    }

    if (source_desc_obj != Py_None) {
        if (!PyObject_TypeCheck(source_desc_obj, Py_TYPE(self))) {
            PyErr_SetString(PyExc_TypeError, "source_desc must be a CFI_cdesc_t or None");
            return NULL;
        }
        source_desc = (PyCFI_cdesc_object *) source_desc_obj;
        source_rank = source_desc->dv.rank;

        if (source_desc->dv.attribute != CFI_attribute_other &&
            source_desc->dv.attribute != CFI_attribute_allocatable &&
            source_desc->dv.attribute != CFI_attribute_pointer) {
            PyErr_SetString(PyExc_ValueError, "Source descriptor attribute must be CFI_attribute_other, CFI_attribute_allocatable, or CFI_attribute_pointer");
            return NULL;
        }

        if (self->dv.rank != source_rank) {
            PyErr_SetString(PyExc_ValueError, "Result descriptor rank must match source descriptor rank");
            return NULL;
        }

        if (self->dv.elem_len == 0) {
            self->dv.elem_len = source_desc->dv.elem_len;
        } else if (self->dv.elem_len != source_desc->dv.elem_len) {
            PyErr_SetString(PyExc_ValueError, "Result descriptor elem_len must match source descriptor elem_len");
            return NULL;
        }

        if (self->dv.type == 0) {
            self->dv.type = source_desc->dv.type;
        } else if (self->dv.type != source_desc->dv.type) {
            PyErr_SetString(PyExc_ValueError, "Result descriptor type must match source descriptor type");
            return NULL;
        }

        if (self->dv.version == 0) {
            self->dv.version = source_desc->dv.version;
        } else if (self->dv.version != source_desc->dv.version) {
            PyErr_SetString(PyExc_ValueError, "Result descriptor version must match source descriptor version");
            return NULL;
        }
    }

    /* Save fields mutated before the call so they can be restored on failure */
    size_t saved_sp_elem_len = self->dv.elem_len;
    CFI_type_t saved_sp_type = self->dv.type;
    int saved_sp_version = self->dv.version;
    CFI_attribute_t saved_sp_attribute = self->dv.attribute;

    if (self->dv.attribute != CFI_attribute_pointer) {
        self->dv.attribute = CFI_attribute_pointer;
    }

    if (source_desc != NULL && source_rank > 0 &&
        lower_bounds_seq != NULL && lower_bounds_seq != Py_None) {
        Py_ssize_t lower_len = PySequence_Length(lower_bounds_seq);
        if (lower_len == -1) {
            return NULL;
        }
        if (lower_len < (Py_ssize_t)source_rank) {
            PyErr_SetString(PyExc_ValueError, "lower_bounds sequence must have at least rank elements");
            return NULL;
        }

        lower_bounds = (CFI_index_t *)malloc(source_rank * sizeof(CFI_index_t));
        if (lower_bounds == NULL) {
            return PyErr_NoMemory();
        }

        for (CFI_rank_t i = 0; i < source_rank; i++) {
            PyObject *lb_item = PySequence_GetItem(lower_bounds_seq, i);
            if (lb_item == NULL) {
                free(lower_bounds);
                return NULL;
            }
            lower_bounds[i] = PyLong_AsLong(lb_item);
            Py_DECREF(lb_item);
            if (PyErr_Occurred()) {
                free(lower_bounds);
                return NULL;
            }
        }
    }

    status = CFI_setpointer(&self->dv,
                            source_desc != NULL ? &source_desc->dv : NULL,
                            lower_bounds);

    if (lower_bounds != NULL) {
        free(lower_bounds);
    }

    if (status != CFI_SUCCESS) {
        /* Restore mutated fields so the descriptor is unchanged on failure */
        self->dv.elem_len = saved_sp_elem_len;
        self->dv.type = saved_sp_type;
        self->dv.version = saved_sp_version;
        self->dv.attribute = saved_sp_attribute;
    }

    return PyLong_FromLong((long)status);
}

static PyMethodDef PyCFI_cdesc_methods[] = {
    {"to_bytes", (PyCFunction) PyCFI_cdesc_to_bytes, METH_NOARGS, "Serialize descriptor to bytes"},
    {"from_bytes", (PyCFunction) PyCFI_cdesc_from_bytes, METH_CLASS|METH_O, "Deserialize descriptor from bytes"},
    {"from_param", (PyCFunction) PyCFI_cdesc_to_bytes, METH_CLASS|METH_O, "ctypes from_param support"},
    {"allocate", (PyCFunction) PyCFI_cdesc_allocate, METH_VARARGS, "Allocate memory for the array descriptor"},
    {"deallocate", (PyCFunction) PyCFI_cdesc_deallocate, METH_NOARGS, "Deallocate memory for the array descriptor"},
    {"establish", (PyCFunction) PyCFI_cdesc_establish, METH_VARARGS, "Establish a descriptor with given properties"},
    {"is_contiguous", (PyCFunction) PyCFI_cdesc_is_contiguous, METH_NOARGS, "Check if the array is contiguous in memory"},
    {"section", (PyCFunction) PyCFI_cdesc_section, METH_VARARGS, "Create a section/subarray of this descriptor"},
    {"select_part", (PyCFunction) PyCFI_cdesc_select_part, METH_VARARGS, "Select a part of a descriptor (for derived types)"},
    {"setpointer", (PyCFunction) PyCFI_cdesc_setpointer, METH_VARARGS, "Set this descriptor to point to another descriptor"},
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
    { Py_tp_dealloc, &PyCFI_cdesc_dealloc},
    { Py_tp_new, &PyCFI_cdesc_new},
    {0, NULL},
};

static PyType_Spec PyCFI_cdesc_spec = {
    .name = "ifb.CFI_cdesc_t",
    .basicsize = sizeof(PyCFI_cdesc_object),
    .itemsize = sizeof(CFI_dim_t),
    .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
    .slots = PyCFI_cdesc_slots,
};


// END CFI_desc setup
/////////////////////////////////////////////////////////


static PyObject* PyIFB_offsetof_cdesc(PyObject *Py_UNUSED(self), PyObject *Py_UNUSED(args)) {
    static const struct {
        const char *name;
        size_t offset;
    } cdesc_offsets[] = {
        {"base_addr", offsetof(CFI_cdesc_t, base_addr)},
        {"elem_len", offsetof(CFI_cdesc_t, elem_len)},
        {"version", offsetof(CFI_cdesc_t, version)},
        {"rank", offsetof(CFI_cdesc_t, rank)},
        {"attribute", offsetof(CFI_cdesc_t, attribute)},
        {"type", offsetof(CFI_cdesc_t, type)},
        {"dim", offsetof(CFI_cdesc_t, dim)},
    };

    PyObject *offsets = PyDict_New();
    if (offsets == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < sizeof(cdesc_offsets) / sizeof(cdesc_offsets[0]); i++) {
        PyObject *value = PyLong_FromSize_t(cdesc_offsets[i].offset);
        if (value == NULL) {
            Py_DECREF(offsets);
            return NULL;
        }
        if (PyDict_SetItemString(offsets, cdesc_offsets[i].name, value) != 0) {
            Py_DECREF(value);
            Py_DECREF(offsets);
            return NULL;
        }
        Py_DECREF(value);
    }

    return offsets;
}

static PyMethodDef IFBModule_methods[] = {
    {"_offsetof_cdesc", (PyCFunction) PyIFB_offsetof_cdesc, METH_NOARGS,
        PyDoc_STR("Return the offsetof values for CFI_cdesc_t fields")},
    {NULL}
};


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

    if (PyModule_AddIntConstant(m, "_sizeof_cdesc", sizeof(CFI_cdesc_t))) {
        return 1;
    } 

    if (PyModule_AddIntConstant(m, "_sizeof_dims", sizeof(CFI_dim_t))) {
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
    #elif REALLY_LLVM
        if (PyModule_AddStringConstant(m, "IFB_COMPILER", "CLANG")) {
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

    // GCC specifics but get defined as -1 in the header

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

    int add_result;

    PyObject *PyCFI_dim_type = PyType_FromSpec(&PyCFI_dim_spec);
    if (PyCFI_dim_type == NULL) {
        return -1;
    }

    add_result = PyModule_AddObjectRef(m, "CFI_dim_t", PyCFI_dim_type);
    if (add_result == -1 ) {
        return add_result;
    }


    PyObject *PyCFI_cdesc_type = PyType_FromSpec(&PyCFI_cdesc_spec);
    if (PyCFI_cdesc_type == NULL) {
        return -1;
    }

    add_result = PyModule_AddObjectRef(m, "CFI_cdesc_t", PyCFI_cdesc_type);
    if (add_result == -1 ) {
        return add_result;
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
    .m_methods = IFBModule_methods,
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
    if (module == NULL) Py_RETURN_NONE;
    if (IFBModule_exec(module) != 0) {
        Py_XDECREF(module);
        Py_RETURN_NONE;
    }
    return module;
#endif
}
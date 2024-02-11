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

    /* Adding module globals */
    if (PyModule_AddIntConstant(m, "CFI_MAX_RANK", CFI_MAX_RANK)) {
        goto except;
    }
    if (PyModule_AddIntConstant(m, "CFI_VERSION", CFI_VERSION)) {
        goto except;
    }
    if (PyModule_AddIntConstant(m, "CFI_attribute_pointer", CFI_attribute_pointer)) {
        goto except;
    }
    if (PyModule_AddIntConstant(m, "CFI_attribute_allocatable", CFI_attribute_allocatable)) {
        goto except;
    }
    if (PyModule_AddIntConstant(m, "CFI_attribute_other", CFI_attribute_other)) {
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
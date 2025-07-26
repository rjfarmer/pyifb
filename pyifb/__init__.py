# SPDX-License-Identifier: GPL-2.0+

from . import ifb
import numpy as np
import ctypes
from collections import namedtuple

__all__ = ["CFI_cdesc"]


class CFI_cdesc:
    def __init__(self, *args, **kwargs):
        self._cfi = None

    def _new(self, rank):
        self._cfi = ifb.CFI_cdesc_t(rank)

    def from_bytes(self, b):
        # Check byte types
        if hasattr(b, "__ctypes_from_outparam__"):
            # for i in range(b._length_):
            #     print(i,b[i])
            b = bytes([b[i] for i in range(b._length_)])

        if isinstance(b, bytearray):
            b = bytes(b)

        print(b)
        self._cfi = ifb.CFI_cdesc_t.from_bytes(b)
        return self

    def to_bytes(self):
        if self._cfi is not None:
            return self._cfi.to_bytes

    @property
    def _as_parameter_(self):
        if self._cfi is not None:
            return self._cfi._as_parameter_

    @classmethod
    def in_dll(cls, lib, name):
        # Make something just big enough so we can pull the rank out
        bytes = ctypes.c_ubyte * ifb._sizeof_cdesc
        temp = cls()
        rank = temp.from_bytes(bytes.in_dll(lib, name)).rank

        print(rank)
        # Get full sized object
        bytes = ctypes.c_ubyte * (ifb._sizeof_cdesc + (ifb._sizeof_dims * rank))
        temp = cls()
        temp = temp.from_bytes(bytes.in_dll(lib, name))

        return temp

    @property
    def value(self):
        if self._cfi is None:
            return None

        if self._cfi.base_addr is None:
            return None

        shape = self.shape()

        ctype = self.ctype
        if ctype is None:
            raise TypeError(f"Can't map to ctype {self.type.name}")

        PTR = ctypes.POINTER(self.ctype)
        x_ptr = ctypes.cast(self._cfi.base_addr, PTR)

        array = np.ctypeslib.as_array(x_ptr, shape=shape)

        dtype = self.dtype
        if dtype is not None:
            array = array.astype(dtype)

        return array

    @value.setter
    def value(self, value):
        pass

    @property
    def rank(self):
        if self._cfi is not None:
            return self._cfi.rank

    @property
    def attribute(self):
        if self._cfi is not None:
            return self._cfi.attribute

    @property
    def dim(self):
        if self._cfi is not None:
            return self._cfi.dim

    @property
    def elem_len(self):
        if self._cfi is not None:
            return self._cfi.elem_len

    @property
    def type(self):
        if self._cfi is not None:
            return _map_cfi_type(self._cfi.type)

    @property
    def version(self):
        if self._cfi is not None:
            return self._cfi.version

    @property
    def shape(self):
        if self._cfi is not None:
            if self.rank == 0:
                return 0
            return tuple([i.extent for i in self._cfi.dim])

    @property
    def size(self):
        if self._cfi is not None:
            return np.product(self.shape)

    @property
    def pytype(self):
        if self._cfi is not None:
            return self.type.pytype

    @property
    def dtype(self):
        if self._cfi is not None:
            return self.type.dtype

    @property
    def ctype(self):
        if self._cfi is not None:
            return _map_cfi_type(self.type).ctype


_cfi_tuple = namedtuple(
    "_cfi_tuple", ("name, ctype, pytype, dtype"), defaults=(None, None, None)
)

_map_cfi_type = {
    ifb.CFI_type_Bool: _cfi_tuple("CFI_type_Bool", ctypes.c_bool, bool, bool),
    ifb.CFI_type_cfunptr: _cfi_tuple("CFI_type_cfunptr", ctypes.c_ubyte * 8),
    ifb.CFI_type_char: _cfi_tuple("CFI_type_char", ctypes.c_char, str, "B"),
    ifb.CFI_type_cptr: _cfi_tuple("CFI_type_cptr", ctypes.c_void_p, None, "V"),
    ifb.CFI_type_double: _cfi_tuple(
        "CFI_type_double", ctypes.c_double, float, np.float64
    ),
    ifb.CFI_type_double_Complex: _cfi_tuple(
        "CFI_type_double_Complex", ctypes.c_double * 2, complex, np.complex128
    ),
    ifb.CFI_type_float: _cfi_tuple("CFI_type_float", ctypes.c_float, float, np.float32),
    ifb.CFI_type_float128: _cfi_tuple("CFI_type_float128", ctypes.c_ubyte * 16),
    ifb.CFI_type_float128_Complex: _cfi_tuple(
        "CFI_type_float128_Complex", ctypes.c_ubyte * 16 * 2
    ),
    ifb.CFI_type_float_Complex: _cfi_tuple(
        "CFI_type_float_Complex", ctypes.c_float * 2, complex, np.complex64
    ),
    ifb.CFI_type_int: _cfi_tuple("CFI_type_int", ctypes.c_int, int, np.int32),
    ifb.CFI_type_int16_t: _cfi_tuple("CFI_type_int16_t", ctypes.c_int16, int, np.int16),
    ifb.CFI_type_int32_t: _cfi_tuple("CFI_type_int32_t", ctypes.c_int32, int, np.int32),
    ifb.CFI_type_int64_t: _cfi_tuple("CFI_type_int64_t", ctypes.c_int64, int, np.int64),
    ifb.CFI_type_int8_t: _cfi_tuple("CFI_type_int8_t", ctypes.c_int8, int, np.int8),
    ifb.CFI_type_long: _cfi_tuple("CFI_type_long", ctypes.c_long, int, np.long),
    ifb.CFI_type_long_double: _cfi_tuple(
        "CFI_type_long_double", ctypes.c_longdouble, int, np.longdouble
    ),
    ifb.CFI_type_long_double_Complex: _cfi_tuple(
        "CFI_type_long_double_Complex", ctypes.c_longdouble * 2
    ),
    ifb.CFI_type_long_long: _cfi_tuple("CFI_type_long_long", ctypes.c_longlong, int),
    ifb.CFI_type_short: _cfi_tuple("CFI_type_short", ctypes.c_short, int, np.short),
    ifb.CFI_type_signed_char: _cfi_tuple(
        "CFI_type_signed_char", ctypes.c_byte, bytes, np.ubyte
    ),
    ifb.CFI_type_struct: _cfi_tuple("CFI_type_struct"),
}

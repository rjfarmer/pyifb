# SPDX-License-Identifier: GPL-2.0+

from .ifb import *
import numpy as np
import ctypes


class CFI_cdesc:
    def __init__(self, rank=0, *args, **kwargs):
        self._rank = 0
        self._cfi = CFI_cdesc_t(self._rank)

    @classmethod
    def from_bytes(cls, bytes):
        pass

    def to_bytes(self):
        return self._cfi.to_bytes

    @property
    def _as_parameter_(self):
        return self._cfi._as_parameter_

    @property
    def value(self):
        if self._cfi.base_addr is None:
            return None

        shape = self.shape()

        PTR = ctypes.POINTER(self.ctype)
        x_ptr = ctypes.cast(self._cfi.base_addr, PTR)

        return np.ctypeslib.as_array(x_ptr, shape=shape)

    @value.setter
    def value(self, value):
        pass

    @property
    def rank(self):
        return self._cfi.rank

    @property
    def attribute(self):
        return self._cfi.attribute

    @property
    def dim(self):
        return self._cfi.dim

    @property
    def elem_len(self):
        return self._cfi.elem_len

    @property
    def type(self):
        return self._cfi.type

    @property
    def version(self):
        return self._cfi.version

    @property
    def shape(self):
        if self.rank == 0:
            return 0
        return tuple([i.extent for i in self._cfi.dim])

    @property
    def size(self):
        return np.product(self.shape)

    @property
    def pytype(self):
        pass

    @property
    def dtype(self):
        pass

    @property
    def ctype(self):
        pass

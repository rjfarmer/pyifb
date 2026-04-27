# SPDX-License-Identifier: GPL-2.0+

import ctypes
import sys
from pathlib import Path

import numpy as np
import pytest

import pyifb as p
import pyifb.utils as utils

lib = ctypes.CDLL(Path("tests", f"bindc.{utils.library_ext()}"))

_ATTR_OFFSET = p.ifb._offsetof_cdesc()["attribute"]
_ATTR_SIZE = p.ifb._sizeof_cfi_attribute_t


def _cdesc_buf(cdesc_t):
    """Return a mutable ctypes buffer from a CFI_cdesc_t with the allocatable attribute set."""
    data = bytearray(cdesc_t.to_bytes())
    data[_ATTR_OFFSET : _ATTR_OFFSET + _ATTR_SIZE] = int(
        p.ifb.CFI_attribute_allocatable
    ).to_bytes(_ATTR_SIZE, sys.byteorder, signed=True)
    return ctypes.create_string_buffer(bytes(data), len(data))


def _element_count(cdesc_t):
    if cdesc_t.rank == 0:
        return 1

    count = 1
    for i in range(cdesc_t.rank):
        count *= cdesc_t.dim[i].extent
    return count


def _int_buffer_values(cdesc_t):
    count = _element_count(cdesc_t)
    ptr = ctypes.cast(cdesc_t.base_addr, ctypes.POINTER(ctypes.c_int))
    return np.ctypeslib.as_array(ptr, shape=(count,)).copy()


class Pair(ctypes.Structure):
    _fields_ = [("x", ctypes.c_int), ("y", ctypes.c_double)]


class TestF90:
    def test_pass_bindc_derived_type(self):
        pair_sum = lib.pair_sum
        pair_sum.restype = ctypes.c_int
        pair_sum.argtypes = [ctypes.POINTER(Pair)]

        assert pair_sum(ctypes.byref(Pair(4, 2.75))) == 6
        assert pair_sum(ctypes.byref(Pair(-3, 5.1))) == 2

    def test_pass_bindc_explicit_array_of_derived_type(self):
        pair_array_sum_explicit = lib.pair_array_sum_explicit
        pair_array_sum_explicit.restype = ctypes.c_int
        pair_array_sum_explicit.argtypes = [ctypes.POINTER(Pair), ctypes.c_int]

        values = (Pair * 3)(Pair(2, 1.9), Pair(-1, 4.2), Pair(5, 0.1))
        assert pair_array_sum_explicit(values, len(values)) == 11

    def test_pass_bindc_allocatable_array_of_derived_type(self):
        pair_array_sum_alloc = lib.pair_array_sum_alloc
        pair_array_sum_alloc.restype = ctypes.c_int

        values = (Pair * 3)(Pair(2, 1.9), Pair(-1, 4.2), Pair(5, 0.1))

        cdesc = p.ifb.CFI_cdesc_t(1)
        status = cdesc.allocate(
            [0],
            [len(values) - 1],
            ctypes.sizeof(Pair),
            p.ifb.CFI_type_struct,
        )
        assert status == p.ifb.CFI_SUCCESS

        ctypes.memmove(cdesc.base_addr, ctypes.addressof(values), ctypes.sizeof(values))
        assert pair_array_sum_alloc(_cdesc_buf(cdesc)) == 11

    def test_make(self):
        a = p.CFI_cdesc(rank=0)
        assert a.rank == 0

    def test_value_setter_auto_allocates(self):
        cdesc = p.CFI_cdesc(rank=1)

        cdesc.value = np.array([3, 1, 4], dtype=np.int32)

        assert cdesc._cfi.base_addr is not None
        assert cdesc.elem_len == 4
        assert cdesc.dim[0].extent == 3
        assert _int_buffer_values(cdesc._cfi).tolist() == [3, 1, 4]

    def test_value_setter_scalar_rank0(self):
        cdesc = p.CFI_cdesc(rank=0)

        cdesc.value = np.array(42, dtype=np.int32)

        assert cdesc.rank == 0
        assert cdesc._cfi.base_addr is not None
        assert cdesc.elem_len == 4
        assert _int_buffer_values(cdesc._cfi).tolist() == [42]

    def test_value_setter_overwrites_existing_buffer(self):
        cdesc = p.CFI_cdesc(rank=1)
        cdesc.value = np.array([1, 2], dtype=np.int32)
        first_addr = cdesc._cfi.base_addr

        cdesc.value = np.array([8, 9], dtype=np.int32)

        assert cdesc._cfi.base_addr == first_addr
        assert _int_buffer_values(cdesc._cfi).tolist() == [8, 9]

    def test_value_setter_rejects_shape_mismatch(self):
        cdesc = p.CFI_cdesc(rank=2)
        cdesc.value = np.zeros((2, 3), dtype=np.int32)

        with pytest.raises(ValueError, match="Shape mismatch"):
            cdesc.value = np.zeros((3, 2), dtype=np.int32)

    def test_value_setter_uses_fortran_layout(self):
        cdesc = p.CFI_cdesc(rank=2)
        cdesc.value = np.array([[1, 2, 3], [4, 5, 6]], dtype=np.int32)

        # Raw memory should be column-major when read linearly.
        assert _int_buffer_values(cdesc._cfi).tolist() == [1, 4, 2, 5, 3, 6]

    def test_add_ints(self):
        add_ints = lib.add_ints
        add_ints.restype = ctypes.c_int
        add_ints.argtypes = [ctypes.c_int, ctypes.c_int]

        assert add_ints(3, 4) == 7
        assert add_ints(-1, 1) == 0
        assert add_ints(0, 0) == 0
        assert add_ints(-10, -5) == -15

    def test_scale_double(self):
        scale_double = lib.scale_double
        scale_double.restype = ctypes.c_double
        scale_double.argtypes = [ctypes.c_double, ctypes.c_double]

        assert abs(scale_double(2.0, 3.0) - 6.0) < 1e-10
        assert abs(scale_double(-1.0, 5.0) - (-5.0)) < 1e-10
        assert abs(scale_double(0.0, 999.0)) < 1e-10
        assert abs(scale_double(1.5, 2.0) - 3.0) < 1e-10

    def test_array_sum_ctypes(self):
        array_sum = lib.array_sum
        array_sum.restype = ctypes.c_int
        array_sum.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int]

        arr = (ctypes.c_int * 5)(1, 2, 3, 4, 5)
        assert array_sum(arr, 5) == 15

        arr_empty = (ctypes.c_int * 0)()
        assert array_sum(arr_empty, 0) == 0

    def test_array_sum_numpy(self):
        array_sum = lib.array_sum
        array_sum.restype = ctypes.c_int
        array_sum.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int]

        arr = np.array([10, 20, 30], dtype=np.int32)
        result = array_sum(arr.ctypes.data_as(ctypes.POINTER(ctypes.c_int)), len(arr))
        assert result == 60

        arr2 = np.arange(1, 11, dtype=np.int32)  # 1..10
        result2 = array_sum(
            arr2.ctypes.data_as(ctypes.POINTER(ctypes.c_int)), len(arr2)
        )
        assert result2 == 55

    def test_fill_array(self):
        lib.fill_array.argtypes = [
            ctypes.POINTER(ctypes.c_int),
            ctypes.c_int,
            ctypes.c_int,
        ]
        lib.fill_array.restype = None

        n = 6
        arr = (ctypes.c_int * n)()
        lib.fill_array(arr, n, 42)
        assert all(arr[i] == 42 for i in range(n))

    def test_fill_array_numpy(self):
        lib.fill_array.argtypes = [
            ctypes.POINTER(ctypes.c_int),
            ctypes.c_int,
            ctypes.c_int,
        ]
        lib.fill_array.restype = None

        arr = np.zeros(8, dtype=np.int32)
        lib.fill_array(arr.ctypes.data_as(ctypes.POINTER(ctypes.c_int)), len(arr), 7)
        assert np.all(arr == 7)

    def test_shift_chars(self):
        lib.shift_chars.argtypes = [
            ctypes.POINTER(ctypes.c_char),
            ctypes.c_int,
            ctypes.c_int,
        ]
        lib.shift_chars.restype = None

        arr = (ctypes.c_char * 4)(b"a", b"b", b"c", b"d")
        lib.shift_chars(arr, len(arr), 1)

        assert bytes(arr) == b"bcde"

    def test_is_alloc_allocated(self):
        """CFI descriptor: allocated array should report as allocated."""
        is_alloc = lib.is_alloc
        is_alloc.restype = ctypes.c_bool

        cdesc = p.ifb.CFI_cdesc_t(1)
        cdesc.allocate([0], [9], 4)  # rank-1, 10 elements of 4 bytes each
        buf = _cdesc_buf(cdesc)
        assert is_alloc(buf) is True

    def test_is_alloc_unallocated(self):
        """CFI descriptor: unallocated array (NULL base_addr) should report as not allocated."""
        is_alloc = lib.is_alloc
        is_alloc.restype = ctypes.c_bool

        cdesc = p.ifb.CFI_cdesc_t(1)  # base_addr is NULL
        buf = _cdesc_buf(cdesc)
        assert is_alloc(buf) is False

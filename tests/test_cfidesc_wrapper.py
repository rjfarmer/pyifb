# SPDX-License-Identifier: GPL-2.0+
"""Tests for the high-level CFI_cdesc Python wrapper (pyifb/__init__.py)."""

import ctypes
import gc
from pathlib import Path

import numpy as np
import pytest

import pyifb as p
import pyifb.utils as utils

# C helper library exposes a pre-initialised rank-1 CFI_cdesc_t at "cdesc_rank1".
_helper_lib = ctypes.CDLL(Path("tests", f"cdesc_test_helper.{utils.library_ext()}"))
_helper_lib.setup.restype = None


def _make_typed_cdesc(
    shape: tuple, cfi_type: int, elem_len: int, arr: np.ndarray
) -> p.CFI_cdesc:
    """Create a CFI_cdesc whose descriptor is established over *arr*'s memory.

    The caller **must** keep *arr* alive for as long as the returned descriptor
    is in use, as no reference to it is retained here.
    """
    rank = len(shape)
    cdesc_t = p.ifb.CFI_cdesc_t(rank)
    status = cdesc_t.establish(
        arr.ctypes.data,
        p.ifb.CFI_attribute_other,
        cfi_type,
        elem_len,
        rank,
        list(shape),
    )
    assert status == p.ifb.CFI_SUCCESS, f"CFI_establish failed (status={status})"
    cdesc = p.CFI_cdesc()
    cdesc._cfi = cdesc_t
    return cdesc


# ---------------------------------------------------------------------------
# from_bytes
# ---------------------------------------------------------------------------


class TestFromBytes:
    def test_plain_bytes(self):
        """from_bytes works with plain bytes (lines 70-71)."""
        src = p.CFI_cdesc(rank=1)
        cdesc = p.CFI_cdesc().from_bytes(src.to_bytes())
        assert cdesc.rank == 1

    def test_bytearray(self):
        """from_bytes converts bytearray to bytes (lines 67-68)."""
        src = p.CFI_cdesc(rank=2)
        cdesc = p.CFI_cdesc().from_bytes(bytearray(src.to_bytes()))
        assert cdesc.rank == 2

    def test_ctypes_array(self):
        """from_bytes handles ctypes arrays via __ctypes_from_outparam__ (lines 62-65)."""
        src = p.CFI_cdesc(rank=3)
        raw = src.to_bytes()
        ct_arr = (ctypes.c_ubyte * len(raw))(*raw)
        cdesc = p.CFI_cdesc().from_bytes(ct_arr)
        assert cdesc.rank == 3


# ---------------------------------------------------------------------------
# to_bytes / _as_parameter_
# ---------------------------------------------------------------------------


class TestSerialize:
    def test_to_bytes_initialized(self):
        """to_bytes returns non-empty bytes when _cfi is set (lines 79-80)."""
        cdesc = p.CFI_cdesc(rank=2)
        b = cdesc.to_bytes()
        assert isinstance(b, bytes)
        assert len(b) > 0

    def test_to_bytes_uninitialized(self):
        """to_bytes returns None when _cfi is None."""
        assert p.CFI_cdesc().to_bytes() is None

    def test_as_parameter_initialized(self):
        """_as_parameter_ is non-None when _cfi is set (lines 89-90)."""
        cdesc = p.CFI_cdesc(rank=1)
        assert cdesc._as_parameter_ is not None

    def test_as_parameter_uninitialized(self):
        """_as_parameter_ is None when _cfi is None."""
        assert p.CFI_cdesc()._as_parameter_ is None


# ---------------------------------------------------------------------------
# in_dll (lines 108-119)
# ---------------------------------------------------------------------------


class TestInDll:
    @pytest.fixture(autouse=True)
    def test_setup(self):
        """Helper library should have a symbol "cdesc_rank1" (lines 108-109)."""
        _helper_lib.setup()

    def test_in_dll_rank(self):
        """in_dll reads the rank from a library symbol (lines 108-119)."""
        cdesc = p.CFI_cdesc.in_dll(_helper_lib, "cdesc_rank1")
        assert cdesc.rank == 1

    def test_in_dll_shape(self):
        """in_dll reads dim extents correctly."""
        cdesc = p.CFI_cdesc.in_dll(_helper_lib, "cdesc_rank1")
        assert cdesc.shape == (3,)

    def test_in_dll_value(self):
        """in_dll descriptor has accessible data values."""
        cdesc = p.CFI_cdesc.in_dll(_helper_lib, "cdesc_rank1")
        result = cdesc.value
        assert result is not None
        assert result.tolist() == [10, 20, 30]

    def test_in_dll_does_not_free_helper_storage(self):
        first = p.CFI_cdesc.in_dll(_helper_lib, "cdesc_rank1")
        assert first.value is not None
        assert first.value.tolist() == [10, 20, 30]

        del first
        gc.collect()

        second = p.CFI_cdesc.in_dll(_helper_lib, "cdesc_rank1")
        result = second.value
        assert result is not None
        assert result.tolist() == [10, 20, 30]


# ---------------------------------------------------------------------------
# value getter (lines 134-155)
# ---------------------------------------------------------------------------


class TestValueGetter:
    def test_cfi_none_returns_none(self):
        """value is None when _cfi has not been set (lines 134-135)."""
        assert p.CFI_cdesc().value is None

    def test_base_addr_none_returns_none(self):
        """value is None when base_addr is NULL (lines 137-138)."""
        cdesc = p.CFI_cdesc(rank=1)
        assert cdesc._cfi.base_addr is None
        assert cdesc.value is None

    def test_returns_array(self):
        """value reconstructs the correct numpy array (lines 140-155)."""
        arr = np.array([10, 20, 30], dtype=np.int32)
        cdesc = _make_typed_cdesc((3,), p.ifb.CFI_type_int, 4, arr)
        result = cdesc.value
        assert result is not None
        assert result.tolist() == [10, 20, 30]

    def test_unmapped_ctype_raises(self):
        """value raises TypeError for types without a ctype mapping (lines 143-144)."""
        arr = np.zeros(8, dtype=np.uint8)
        cdesc = _make_typed_cdesc((1,), p.ifb.CFI_type_struct, 8, arr)
        with pytest.raises(TypeError, match="Can't map to ctype"):
            _ = cdesc.value


# ---------------------------------------------------------------------------
# value setter edge cases (lines 160, 163)
# ---------------------------------------------------------------------------


class TestValueSetter:
    def test_noop_when_cfi_none(self):
        """value setter is a no-op when _cfi is None (line 160)."""
        cdesc = p.CFI_cdesc()
        assert cdesc._cfi is None
        cdesc.value = np.array([1, 2, 3])
        assert cdesc._cfi is None  # unchanged

    def test_converts_list_to_ndarray(self):
        """value setter accepts a plain list and converts it (line 163)."""
        cdesc = p.CFI_cdesc(rank=1)
        cdesc.value = [1, 2, 3]
        assert cdesc._cfi.base_addr is not None
        assert cdesc._cfi.dim[0].extent == 3


# ---------------------------------------------------------------------------
# Properties on an uninitialised wrapper (returns None)
# ---------------------------------------------------------------------------


class TestPropertiesUninitialized:
    def _blank(self) -> p.CFI_cdesc:
        return p.CFI_cdesc()

    def test_rank_none(self):
        assert self._blank().rank is None

    def test_attribute_none(self):
        assert self._blank().attribute is None

    def test_dim_none(self):
        assert self._blank().dim is None

    def test_elem_len_none(self):
        assert self._blank().elem_len is None

    def test_type_none(self):
        assert self._blank().type is None

    def test_version_none(self):
        assert self._blank().version is None

    def test_shape_none(self):
        assert self._blank().shape is None

    def test_size_none(self):
        assert self._blank().size is None

    def test_pytype_none(self):
        assert self._blank().pytype is None

    def test_dtype_none(self):
        assert self._blank().dtype is None

    def test_ctype_none(self):
        assert self._blank().ctype is None


# ---------------------------------------------------------------------------
# Properties on an initialised, typed descriptor
# ---------------------------------------------------------------------------


class TestPropertiesInitialized:
    @pytest.fixture
    def cdesc_int32(self):
        arr = np.array([1, 2, 3], dtype=np.int32)
        cdesc = _make_typed_cdesc((3,), p.ifb.CFI_type_int, 4, arr)
        # Keep arr alive for the duration of this fixture
        cdesc._test_arr = arr
        return cdesc

    def test_attribute(self, cdesc_int32):
        """attribute is readable (lines 191-192)."""
        assert isinstance(cdesc_int32.attribute, int)

    def test_version(self, cdesc_int32):
        """version matches CFI_VERSION (lines 215-216)."""
        assert cdesc_int32.version == p.ifb.CFI_VERSION

    def test_size(self):
        """size returns total element count (lines 229-230)."""
        cdesc = p.CFI_cdesc(rank=2)
        cdesc.value = np.zeros((3, 4), dtype=np.int32)
        assert cdesc.size == 12

    def test_pytype(self, cdesc_int32):
        """pytype returns the mapped Python type (lines 235-236)."""
        assert cdesc_int32.pytype is int

    def test_ctype(self, cdesc_int32):
        """ctype returns the mapped ctypes type (lines 247-248)."""
        assert cdesc_int32.ctype is ctypes.c_int

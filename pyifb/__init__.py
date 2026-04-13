# SPDX-License-Identifier: GPL-2.0+
from __future__ import annotations

from . import ifb  # type: ignore[attr-defined]
import struct
import numpy as np
import ctypes
from collections import namedtuple
from typing import Any, Type

# Byte offset of the rank field inside CFI_cdesc_t.
# Layout: void* base_addr + size_t elem_len + int version + int8_t rank
_RANK_OFFSET: int = (
    ctypes.sizeof(ctypes.c_void_p)
    + ctypes.sizeof(ctypes.c_size_t)
    + ctypes.sizeof(ctypes.c_int)
)

__all__ = ["CFI_cdesc", "ifb"]


class CFI_cdesc:
    """High-level Python wrapper for ISO Fortran Binding C Descriptors (CFI_cdesc_t).

    This class provides a Pythonic interface to Fortran C descriptors, allowing
    interaction with Fortran arrays and data objects through the ISO_Fortran_binding.h
    C interoperability specification. It handles serialization, deserialization, and
    provides access to descriptor properties like rank, type, shape, and data.

    Attributes:
        _cfi: Internal CFI_cdesc_t C descriptor object.

    Examples:
        >>> cdesc = CFI_cdesc()
        >>> cdesc._new(2)  # Create a rank-2 descriptor
        >>> array = cdesc.value  # Access as numpy array
    """

    _cfi: Any

    def __init__(self, rank: int | None = None, *args: Any, **kwargs: Any) -> None:
        """Initialize an empty CFI_cdesc wrapper.

        The internal C descriptor is not allocated until _new() or from_bytes() is called.
        """
        self._cfi = None
        if rank is not None:
            self._new(rank)

    def _new(self, rank: int) -> None:
        """Create a new C descriptor with the specified rank.

        Args:
            rank (int): Number of dimensions (0 for scalar, max CFI_MAX_RANK).
        """
        self._cfi = ifb.CFI_cdesc_t(rank)

    def from_bytes(self, b: Any) -> CFI_cdesc:
        """Deserialize a C descriptor from bytes.

        Accepts bytes from various sources (ctypes arrays, bytearrays, raw bytes)
        and populates the internal C descriptor.

        Args:
            b: Byte-like object containing serialized CFI_cdesc_t data.

        Returns:
            self: For method chaining.
        """
        # Check byte types
        if hasattr(b, "__ctypes_from_outparam__"):
            # for i in range(b._length_):
            #     print(i,b[i])
            b = bytes([b[i] for i in range(b._length_)])

        if isinstance(b, bytearray):
            b = bytes(b)

        self._cfi = ifb.CFI_cdesc_t.from_bytes(b)
        return self

    def to_bytes(self) -> bytes | None:
        """Serialize the C descriptor to bytes.

        Returns:
            bytes: Serialized representation of the descriptor including rank and dimensions.
        """
        if self._cfi is not None:
            return self._cfi.to_bytes()
        return None

    @property
    def _as_parameter_(self) -> bytes | None:
        """ctypes interoperability property for passing to C functions.

        Returns:
            bytes: Serialized descriptor for use with ctypes C function calls.
        """
        if self._cfi is not None:
            return self._cfi._as_parameter
        return None

    @classmethod
    def in_dll(cls, lib: ctypes.CDLL, name: str) -> CFI_cdesc:
        """Load a C descriptor from a shared library (DLL/SO).

        Reads a descriptor from a named global variable in a loaded library.
        First reads just the base descriptor to determine rank, then reads the
        full descriptor including dimension information.

        Args:
            lib: ctypes CDLL library object.
            name (str): Name of the descriptor variable in the library.

        Returns:
            CFI_cdesc: Populated descriptor object.
        """
        # Read just the base header to extract rank without going through
        # from_bytes (which requires the full header + all dim entries).
        header_type = ctypes.c_ubyte * ifb._sizeof_cdesc
        raw_header = header_type.in_dll(lib, name)
        header_bytes = bytes([raw_header[i] for i in range(ifb._sizeof_cdesc)])
        rank = struct.unpack_from("b", header_bytes, _RANK_OFFSET)[0]

        # Now read the full descriptor including dim entries.
        full_size = ifb._sizeof_cdesc + ifb._sizeof_dims * rank
        full_type = ctypes.c_ubyte * full_size
        raw_full = full_type.in_dll(lib, name)
        full_bytes = bytes([raw_full[i] for i in range(full_size)])

        temp = cls()
        temp._cfi = ifb.CFI_cdesc_t.from_bytes(full_bytes)
        return temp

    @property
    def value(self) -> np.ndarray | None:
        """Reconstruct the array data as a numpy array.

        Casts the base_addr pointer to the appropriate ctypes type and creates
        a numpy array view with the descriptor's shape and dtype.

        Returns:
            numpy.ndarray: Array view of the data, or None if uninitialized.

        Raises:
            TypeError: If the descriptor's type cannot be mapped to a ctypes type.
        """
        if self._cfi is None:
            return None

        if self._cfi.base_addr is None:
            return None

        shape = self.shape

        ctype = self.ctype
        if ctype is None:
            cfi_type = self.type
            type_name = cfi_type.name if cfi_type is not None else "unknown"
            raise TypeError(f"Can't map to ctype {type_name}")

        PTR = ctypes.POINTER(self.ctype)
        x_ptr = ctypes.cast(self._cfi.base_addr, PTR)

        array = np.ctypeslib.as_array(x_ptr, shape=shape)

        dtype = self.dtype
        if dtype is not None:
            array = array.astype(dtype)

        return array

    @value.setter
    def value(self, value: Any) -> None:
        if self._cfi is None:
            return

        if not isinstance(value, np.ndarray):
            value = np.asarray(value)

        if self._cfi.base_addr is None:
            lower = [0] * value.ndim
            upper = [s - 1 for s in value.shape]
            self._cfi.allocate(lower, upper, value.itemsize)

        shape = self.shape
        if shape != 0 and value.shape != shape:
            raise ValueError(f"Shape mismatch: expected {shape}, got {value.shape}")

        try:
            dtype = self.dtype
        except (TypeError, KeyError):
            dtype = None

        src = np.asfortranarray(value.astype(dtype) if dtype is not None else value)
        ctypes.memmove(self._cfi.base_addr, src.ctypes.data, src.nbytes)

    @property
    def rank(self) -> int | None:
        """int: Number of dimensions (0 for scalar, >0 for array)."""
        if self._cfi is not None:
            return self._cfi.rank
        return None

    @property
    def attribute(self) -> int | None:
        """int: Attribute code (allocatable, pointer, or other)."""
        if self._cfi is not None:
            return self._cfi.attribute
        return None

    @property
    def dim(self) -> tuple[Any, ...] | None:
        """tuple or None: Dimension objects containing lower_bound, extent, and stride info."""
        if self._cfi is not None:
            return self._cfi.dim
        return None

    @property
    def elem_len(self) -> int | None:
        """int: Size of one array element in bytes."""
        if self._cfi is not None:
            return self._cfi.elem_len
        return None

    @property
    def type(self) -> _cfi_tuple | None:
        """NamedTuple: Type information (name, ctype, pytype, dtype)."""
        if self._cfi is not None:
            return _map_cfi_type[self._cfi.type]
        return None

    @property
    def version(self) -> int | None:
        """int: CFI version of the descriptor."""
        if self._cfi is not None:
            return self._cfi.version
        return None

    @property
    def shape(self) -> int | tuple[int, ...] | None:
        """tuple or int: Shape of the array (0 for scalar, tuple of extents for arrays)."""
        if self._cfi is not None:
            if self.rank == 0:
                return 0
            return tuple([i.extent for i in self._cfi.dim])
        return None

    @property
    def size(self) -> int | None:
        """int: Total number of elements in the array."""
        shape = self.shape
        if shape is None:
            return None
        if shape == 0:
            return 0
        return int(np.prod(shape))

    @property
    def pytype(self) -> Type[Any] | None:
        """type: Python type corresponding to the element type (int, float, bool, etc)."""
        cfi_type = self.type
        if cfi_type is not None:
            return cfi_type.pytype
        return None

    @property
    def dtype(self) -> np.dtype[Any] | str | None:
        """numpy.dtype or str or None: NumPy dtype for the element type."""
        cfi_type = self.type
        if cfi_type is not None:
            return cfi_type.dtype
        return None

    @property
    def ctype(self) -> Type[Any] | None:
        """ctypes type: ctypes representation of the element type for C interop."""
        cfi_type = self.type
        if cfi_type is not None:
            return cfi_type.ctype
        return None


_cfi_tuple = namedtuple(
    "_cfi_tuple", ("name", "ctype", "pytype", "dtype"), defaults=(None, None, None)
)

_map_cfi_type: dict[int, _cfi_tuple] = {
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

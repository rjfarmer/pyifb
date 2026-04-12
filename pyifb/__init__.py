# SPDX-License-Identifier: GPL-2.0+

from . import ifb
import numpy as np
import ctypes
from collections import namedtuple

__all__ = ["CFI_cdesc"]


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

    def __init__(self, rank=None, *args, **kwargs):
        """Initialize an empty CFI_cdesc wrapper.

        The internal C descriptor is not allocated until _new() or from_bytes() is called.
        """
        self._cfi = None
        if rank is not None:
            self._new(rank)

    def _new(self, rank):
        """Create a new C descriptor with the specified rank.

        Args:
            rank (int): Number of dimensions (0 for scalar, max CFI_MAX_RANK).
        """
        self._cfi = ifb.CFI_cdesc_t(rank)

    def from_bytes(self, b):
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

    def to_bytes(self):
        """Serialize the C descriptor to bytes.

        Returns:
            bytes: Serialized representation of the descriptor including rank and dimensions.
        """
        if self._cfi is not None:
            return self._cfi.to_bytes

    @property
    def _as_parameter_(self):
        """ctypes interoperability property for passing to C functions.

        Returns:
            bytes: Serialized descriptor for use with ctypes C function calls.
        """
        if self._cfi is not None:
            return self._cfi._as_parameter_

    @classmethod
    def in_dll(cls, lib, name):
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
        # Make something just big enough so we can pull the rank out
        descriptor_bytes = ctypes.c_ubyte * ifb._sizeof_cdesc
        temp = cls()
        rank = temp.from_bytes(descriptor_bytes.in_dll(lib, name)).rank

        # Get full sized object
        descriptor_bytes = ctypes.c_ubyte * (
            ifb._sizeof_cdesc + (ifb._sizeof_dims * rank)
        )
        temp = cls()
        temp = temp.from_bytes(descriptor_bytes.in_dll(lib, name))

        return temp

    @property
    def value(self):
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
    def rank(self):
        """int: Number of dimensions (0 for scalar, >0 for array)."""
        if self._cfi is not None:
            return self._cfi.rank

    @property
    def attribute(self):
        """int: Attribute code (allocatable, pointer, or other)."""
        if self._cfi is not None:
            return self._cfi.attribute

    @property
    def dim(self):
        """tuple or None: Dimension objects containing lower_bound, extent, and stride info."""
        if self._cfi is not None:
            return self._cfi.dim

    @property
    def elem_len(self):
        """int: Size of one array element in bytes."""
        if self._cfi is not None:
            return self._cfi.elem_len

    @property
    def type(self):
        """NamedTuple: Type information (name, ctype, pytype, dtype)."""
        if self._cfi is not None:
            return _map_cfi_type(self._cfi.type)

    @property
    def version(self):
        """int: CFI version of the descriptor."""
        if self._cfi is not None:
            return self._cfi.version

    @property
    def shape(self):
        """tuple or int: Shape of the array (0 for scalar, tuple of extents for arrays)."""
        if self._cfi is not None:
            if self.rank == 0:
                return 0
            return tuple([i.extent for i in self._cfi.dim])

    @property
    def size(self):
        """int: Total number of elements in the array."""
        if self._cfi is not None:
            return np.product(self.shape)

    @property
    def pytype(self):
        """type: Python type corresponding to the element type (int, float, bool, etc)."""
        if self._cfi is not None:
            return self.type.pytype

    @property
    def dtype(self):
        """numpy.dtype or str or None: NumPy dtype for the element type."""
        if self._cfi is not None:
            return self.type.dtype

    @property
    def ctype(self):
        """ctypes type: ctypes representation of the element type for C interop."""
        if self._cfi is not None:
            return _map_cfi_type(self.type).ctype


_cfi_tuple = namedtuple(
    "_cfi_tuple", ("name", "ctype", "pytype", "dtype"), defaults=(None, None, None)
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

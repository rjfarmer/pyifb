
# pyifb

Python bindings for Fortran's `ISO_Fortran_binding.h` descriptors (`CFI_cdesc_t`).

`pyifb` provides:

- A low-level extension module (`pyifb.ifb`) exposing C descriptor structs, constants, and CFI helper functions.
- A high-level Python wrapper (`pyifb.CFI_cdesc`) that makes descriptor allocation and array exchange feel NumPy-native.

This is useful when interoperating with Fortran code that uses `bind(C)` and assumed-shape/descriptor-based interfaces.

## Features

- Wraps `CFI_cdesc_t` and `CFI_dim_t` from `ISO_Fortran_binding.h`
- Exposes CFI constants/macros (for example: `CFI_SUCCESS`, `CFI_type_int`, `CFI_MAX_RANK`)
- Supports creating, serializing, and deserializing descriptors
- Supports descriptor operations such as allocation, deallocation, sectioning, pointer setup, and contiguity checks
- Includes a high-level `CFI_cdesc.value` property for exchanging data with NumPy arrays

## Requirements

- Python 3.10+
- NumPy (installed automatically as a dependency)
- A C compiler toolchain for building the extension
- A Fortran runtime available at link/runtime
	- Default build path targets GNU Fortran runtime (`libgfortran`)
	- Alternative compiler/runtime behavior is handled in `setup.py`

## Install

Install from source:

```bash
python -m pip install .
```

For editable development install:

```bash
python -m pip install -e .
```

## Build

Build wheel/sdist:

```bash
python -m build
```

## Quick Start

### High-level descriptor wrapper

```python
import numpy as np
import pyifb

# Create rank-2 descriptor
cdesc = pyifb.CFI_cdesc(rank=2)

# Assign NumPy data (allocates descriptor storage if needed)
cdesc.value = np.array([[1, 2, 3], [4, 5, 6]], dtype=np.int32)

print(cdesc.rank)      # 2
print(cdesc.shape)     # (2, 3)
print(cdesc.elem_len)  # 4

# Round-trip back to NumPy
arr = cdesc.value
print(arr)
```

### Low-level API access

```python
import pyifb

# Access raw constants and structs from the extension
print(pyifb.ifb.CFI_MAX_RANK)
print(pyifb.ifb.CFI_SUCCESS)

cdesc_t = pyifb.ifb.CFI_cdesc_t(rank=1)
status = cdesc_t.allocate([0], [9], 4)
print(status == pyifb.ifb.CFI_SUCCESS)
```

## Using `pyifb` With Fortran Code

`pyifb` is designed to work with Fortran procedures exposed through
`bind(C)`. The typical workflow is:

1. Write a `bind(C)` Fortran routine.
2. Build it as a shared library (`.so`/`.dylib`/`.dll`).
3. Create a `CFI_cdesc` in Python and pass its serialized descriptor to the
   Fortran routine with `ctypes`.

### Fortran side (`bind(C)` + assumed-shape)

```fortran
module finterop
  use iso_c_binding
  implicit none
contains

  integer(c_int) function sum_desc(x) bind(C, name="sum_desc")
    integer(c_int), intent(in) :: x(:)
    integer(c_int) :: i

    sum_desc = 0
    do i = 1, size(x)
      sum_desc = sum_desc + x(i)
    end do
  end function sum_desc

end module finterop
```

Build a shared library (GNU example):

```bash
gfortran -shared -fPIC -O2 finterop.f90 -o libfinterop.so
```

### Python side (`pyifb` + `ctypes`)

```python
import ctypes
import numpy as np
import pyifb

lib = ctypes.CDLL("./libfinterop.so")
sum_desc = lib.sum_desc
sum_desc.restype = ctypes.c_int
sum_desc.argtypes = [ctypes.c_void_p]

# Build a rank-1 descriptor and populate from NumPy.
cdesc = pyifb.CFI_cdesc(rank=1)
cdesc.value = np.array([10, 20, 30], dtype=np.int32)

# Pass descriptor bytes by pointer.
raw = cdesc.to_bytes()
buf = ctypes.create_string_buffer(raw, len(raw))

result = sum_desc(buf)
print(result)  # 60
```

### Notes and pitfalls

- Use Fortran interoperable kinds (`integer(c_int)`, `real(c_double)`, etc.)
  from `iso_c_binding`.
- Match NumPy dtype to Fortran kind (for example, `np.int32` <-> `integer(c_int)`).
- `CFI_cdesc.value` stores arrays in Fortran (column-major) order.
- For scalar descriptors, use `CFI_cdesc(rank=0)` and assign a scalar NumPy value.
- For routines that expect allocatable/pointer dummy arguments, descriptor
  attributes must also match (`CFI_attribute_allocatable`/
  `CFI_attribute_pointer`).

## Testing

The test suite builds a small Fortran shared library in `tests/` (via CMake) and then runs `pytest`.

Run tests:

```bash
pytest
```

Or use the helper script:

```bash
./build_and_test.sh
```

## Development

Useful scripts in the repository root:

- `build.sh`
- `build_and_test.sh`
- `lint.sh`
- `coverage.sh`

Optional dependency groups (from `pyproject.toml`):

- `test`
- `coverage`
- `dev`
- `perfomance`

Install development extras (example):

```bash
python -m pip install -e ".[dev,test]"
```

## Compilers

By default we compile with gcc/gfortran and that is the version shipped in PyPI. We support
other compiliers:

- Intel ICC/IFX
- Clang

These must be built locally, to do set the environement variables:

````bash
export CC=icx
export FC=ifx
````

or

````bash
export CC=clang
export FC=flang
````

Then install the package

```bash
python -m pip install -e .
```

Other compilers can be support on request, if they have a publicly readable `ISO_Fortran_Bindings.h` file. Please open a bug request for a new compiler.


## License

GPL-2.0-or-later. See `COPYING.txt`.

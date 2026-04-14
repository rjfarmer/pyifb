
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

## Testing

The test suite builds a small Fortran shared library in `tests/` (via `make`) and then runs `pytest`.

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

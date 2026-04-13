
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

## Troubleshooting

### Build fails with missing compiler toolchain

Symptoms:

- `error: command 'gcc' failed`
- `fatal error: Python.h: No such file or directory`

Fix:

- Install system build tools and Python development headers.
- On Debian/Ubuntu this is typically:

```bash
sudo apt-get update
sudo apt-get install -y build-essential python3-dev
```

### Import or test fails with missing Fortran runtime

Symptoms:

- `ImportError: libgfortran.so.*: cannot open shared object file`

Fix:

- Install GNU Fortran runtime/development package for your distro.
- Confirm runtime visibility with:

```bash
ldconfig -p | grep gfortran
```

### Tests fail because Fortran test library was not built

Symptoms:

- `OSError` when loading `tests/bindc.so` (or `.dylib`/`.dll`)

Fix:

- Ensure `gfortran` is installed.
- Rebuild test fixtures:

```bash
make -C tests clean
make -C tests
pytest
```

### Using `CC=clang` and link errors appear

Symptoms:

- Linker errors mentioning `FortranRuntime` or missing Fortran symbols

Fix:

- This project has special-case clang behavior in `setup.py` and may require LLVM Fortran runtime availability.
- If clang-based linking is problematic on your system, unset `CC` and build with default GCC toolchain:

```bash
unset CC
python -m pip install -e .
```

### Intel toolchain (`CC=icx`) fails

Symptoms:

- Link errors related to Intel runtime paths

Fix:

- Ensure oneAPI is installed and `ONEAPI_ROOT` is exported.
- Verify the expected runtime library directory exists.

### ABI/environment mismatch after Python upgrade

Symptoms:

- Extension builds but import fails, or stale binary behavior appears

Fix:

- Recreate your virtual environment and reinstall:

```bash
python -m venv .venv
source .venv/bin/activate
python -m pip install -U pip
python -m pip install -e ".[dev,test]"
```

### Need a clean rebuild

```bash
rm -rf build dist *.egg-info
find . -name "__pycache__" -type d -prune -exec rm -rf {} +
python -m pip install -e .
```

## Project Layout

- `pyifb/ifb.c`: CPython extension implementation
- `pyifb/ifb.h`: extension declarations/helpers
- `pyifb/__init__.py`: public Python API and high-level wrapper
- `tests/`: pytest suite and Fortran test fixture (`bindc.f90`)

## License

GPL-2.0-or-later. See `COPYING.txt`.

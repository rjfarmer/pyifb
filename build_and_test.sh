#!/bin/bash

set -euo pipefail

python -m pip install -e .
# Ensure local tests exercise the current C source, not a stale built extension.
python setup.py build_ext --inplace
python -m pytest

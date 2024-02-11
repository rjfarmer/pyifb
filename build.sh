#!/bin/bash

rm -rf *.so build dist pyifb.egg-info/
python -m build && python -m pip install --force-reinstall dist/pyifb*.whl

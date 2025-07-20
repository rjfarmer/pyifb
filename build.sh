#!/bin/bash

rm dist/*
python -m pip uninstall pyifb -y
python -m build
python -m pip install dist/pyifb*.whl

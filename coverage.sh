#!/usr/bin/bash

export PYTEST_COVERAGE=1
python -m pytest --cov=pyifb --cov-report html 
xdg-open htmlcov/index.html

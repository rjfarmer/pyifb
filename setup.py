from setuptools import setup, Extension
import pathlib
import functools

setup(
    ext_modules=[
        Extension(
            name="pyifb.ifb",
            sources=[
                "pyifb/ifb.c",
            ],
        )
    ],
)

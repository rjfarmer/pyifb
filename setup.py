from setuptools import setup, Extension
import pathlib
import functools

@functools.cache
def get_include():
    p = list(pathlib.Path('/usr').glob('**/ISO_Fortran_binding.h'))[0]
    return p.parent


setup(
    ext_modules=[
        Extension(
            name = "pyifb.ifb",
            sources = ["pyifb/ifb.c",],
            include_dirs = [get_include()]
        )
    ],
)

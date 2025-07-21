from setuptools import setup, Extension


setup(
    ext_modules=[
        Extension(
            name="pyifb.ifb",
            sources=[
                "pyifb/ifb.c",
            ],
            libraries=["gfortran"],
            extra_compile_args=[
                "-ggdb",
                "-O",
                "-fno-eliminate-unused-debug-symbols",
                "-fvar-tracking-assignments",
            ],
            py_limited_api=True,
        )
    ],
)

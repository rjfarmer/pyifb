from setuptools import setup, Extension



setup(
    ext_modules=[
        Extension(
            name="pyifb.ifb",
            sources=["pyifb/ifb.c",],
        )
    ],
)

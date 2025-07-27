# SPDX-License-Identifier: GPL-2.0+

import platform


def library_ext():
    """
    Determine shared library extension for a current os_platform
    """
    os_platform = platform.system()
    if os_platform == "Darwin":
        return "dylib"
    elif os_platform == "Windows" or "CYGWIN" in os_platform:
        return "dll"
    else:
        return "so"

"""
Copyright (c) Contributors to the Open 3D Engine Project

SPDX-License-Identifier: Apache-2.0 OR MIT
"""


def init():
    import os
    import sys

    sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../automatedtesting_shared")
    sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/../EditorPythonTestTools/editor_python_test_tools")

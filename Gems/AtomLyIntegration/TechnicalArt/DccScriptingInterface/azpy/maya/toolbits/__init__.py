# coding:utf-8
#!/usr/bin/python
#
# Copyright (c) Contributors to the Open 3D Engine Project
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#
# -- This line is 75 characters -------------------------------------------
# The __init__.py files help guide import statements without automatically
# importing all of the modules
"""azpy.maya.toolbits.__init__"""

mport os

from azpy.env_bool import env_bool
from azpy.constants import ENVAR_DCCSI_GDEBUG
from azpy.constants import ENVAR_DCCSI_DEV_MODE

#  global space
_G_DEBUG = env_bool(ENVAR_DCCSI_GDEBUG, False)
_DCCSI_DEV_MODE = env_bool(ENVAR_DCCSI_DEV_MODE, False)

_PACKAGENAME = __name__
if _PACKAGENAME is '__main__':
    _PACKAGENAME = 'azpy.maya.toolbits'

import azpy
_LOGGER = azpy.initialize_logger(_PACKAGENAME)
_LOGGER.debug('Invoking __init__.py for {0}.'.format({_PACKAGENAME}))

__all__ = ['detach']

del _LOGGER
#--------------------------------------------------------------------------


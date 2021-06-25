#
# Copyright (c) Contributors to the Open 3D Engine Project
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

ly_set(LY_PLATFORM_DETECTION_Android Android)
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Darwin)
    ly_set(LY_HOST_PLATFORM_DETECTION_Android Mac)
else()
    ly_set(LY_HOST_PLATFORM_DETECTION_Android ${CMAKE_HOST_SYSTEM_NAME})
endif()

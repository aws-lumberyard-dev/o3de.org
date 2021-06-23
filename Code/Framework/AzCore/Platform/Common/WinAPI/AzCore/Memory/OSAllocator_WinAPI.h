/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#define AZ_OS_MALLOC(byteSize, alignment) _aligned_malloc(byteSize, alignment)
#define AZ_OS_FREE(pointer) _aligned_free(pointer)

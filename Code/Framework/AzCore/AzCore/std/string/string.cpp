/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#define AZ_STRING_EXPLICIT_SPECIALIZATION
#include <AzCore/std/string/string.h>
#undef AZ_STRING_EXPLICIT_SPECIALIZATION

namespace AZStd
{
    // explicit specialization
    template class basic_string<char>;
    //template class basic_string<wchar_t>;
}

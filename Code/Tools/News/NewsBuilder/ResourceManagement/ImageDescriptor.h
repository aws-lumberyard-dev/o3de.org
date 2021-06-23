/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <NewsShared/ResourceManagement/Descriptor.h>

class QString;

namespace News
{
    class ImageDescriptor
        : public Descriptor
    {
    public:
        explicit ImageDescriptor(Resource& resource);

        bool Read(const QString& filename, QString& error) const;
    };
}

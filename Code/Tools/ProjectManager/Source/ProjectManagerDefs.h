/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <QString>

namespace O3DE::ProjectManager
{
    inline constexpr static int ProjectPreviewImageWidth = 210;
    inline constexpr static int ProjectPreviewImageHeight = 280;

    static const QString ProjectBuildPathPostfix = "Windows_VS2019";
    static const QString ProjectBuildErrorLogPathPostfix = "CMakeFiles/CMakeProjectBuildError.log";
    static const QString ProjectPreviewImagePath = "preview.png";
} // namespace O3DE::ProjectManager

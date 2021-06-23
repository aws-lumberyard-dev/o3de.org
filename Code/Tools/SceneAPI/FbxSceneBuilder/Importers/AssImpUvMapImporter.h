/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <SceneAPI/FbxSceneBuilder/ImportContexts/AssImpImportContexts.h>
#include <SceneAPI/SceneCore/Components/LoadingComponent.h>

namespace AZ
{
    namespace SceneAPI
    {
        namespace FbxSceneBuilder
        {
            class AssImpUvMapImporter
                : public SceneCore::LoadingComponent
            {
            public:
                AZ_COMPONENT(AssImpUvMapImporter, "{BF02F231-848B-4CDB-9B11-55EEE15CFAA6}", SceneCore::LoadingComponent);

                AssImpUvMapImporter();
                ~AssImpUvMapImporter() override = default;

                static void Reflect(ReflectContext* context);

                Events::ProcessingResult ImportUvMaps(AssImpSceneNodeAppendedContext& context);

            protected:
                static const char* m_defaultNodeName;
            };
        } // namespace FbxSceneBuilder
    } // namespace SceneAPI
} // namespace AZ

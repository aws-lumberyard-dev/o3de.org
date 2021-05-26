/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "TextureAtlas_precompiled.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/IO/FileIO.h>
#include <AzFramework/API/ApplicationAPI.h>

#include "TextureAtlasSystemComponent.h"
#include "TextureAtlasImpl.h"
#include <AzCore/Asset/AssetManagerBus.h>
#include <AzFramework/StringFunc/StringFunc.h>

#include <Atom/RPI.Public/Image/StreamingImage.h>

namespace
{
    AZ::Data::Instance<AZ::RPI::Image> LoadAtlasImage(const AZStd::string& imagePath)
    {
        // The file may not be in the AssetCatalog at this point if it is still processing or doesn't exist on disk.
        // Use GenerateAssetIdTEMP instead of GetAssetIdByPath so that it will return a valid AssetId anyways
        AZ::Data::AssetId streamingImageAssetId;
        AZ::Data::AssetCatalogRequestBus::BroadcastResult(
            streamingImageAssetId, &AZ::Data::AssetCatalogRequestBus::Events::GenerateAssetIdTEMP,
            imagePath.c_str());

        streamingImageAssetId.m_subId = AZ::RPI::StreamingImageAsset::GetImageAssetSubId();
        auto streamingImageAsset = AZ::Data::AssetManager::Instance().FindOrCreateAsset<AZ::RPI::StreamingImageAsset>(streamingImageAssetId, AZ::Data::AssetLoadBehavior::PreLoad);
        AZ::Data::Instance<AZ::RPI::Image> image = AZ::RPI::StreamingImage::FindOrCreate(streamingImageAsset);
        return image;
    }
}

namespace TextureAtlasNamespace
{
    void TextureAtlasSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<TextureAtlasSystemComponent, AZ::Component>()
                ->Version(0)
                ->Attribute(AZ::Edit::Attributes::SystemComponentTags, AZStd::vector<AZ::Crc32>({ AZ_CRC("AssetBuilder", 0xc739c7d7) }));
            ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<TextureAtlasSystemComponent>(
                    "TextureAtlas", "This component loads and manages TextureAtlases")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
            }
        }
        TextureAtlasImpl::Reflect(context);
    }

    void TextureAtlasSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("TextureAtlasService"));
    }

    void
        TextureAtlasSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("TextureAtlasService"));
    }

    void TextureAtlasSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        (void)required;
    }

    void TextureAtlasSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void TextureAtlasSystemComponent::Init() { }

    void TextureAtlasSystemComponent::Activate()
    {
        TextureAtlasRequestBus::Handler::BusConnect();
        AzFramework::AssetCatalogEventBus::Handler::BusConnect();
    }

    void TextureAtlasSystemComponent::Deactivate()
    {
        TextureAtlasRequestBus::Handler::BusDisconnect();
        AzFramework::AssetCatalogEventBus::Handler::BusDisconnect();
    }

    void TextureAtlasSystemComponent::OnCatalogAssetChanged(const AZ::Data::AssetId& assetId)
    {
        for (auto iterator = m_atlases.begin(); iterator != m_atlases.end(); ++iterator)
        {
            if (iterator->second.m_atlasAssetId == assetId)
            {
                TextureAtlasImpl* input = AZ::Utils::LoadObjectFromFile<TextureAtlasImpl>(iterator->second.m_path);
                reinterpret_cast<TextureAtlasImpl*>(iterator->second.m_atlas)->OverwriteMappings(input);
                delete input;

                // We reload the image here to prevent stuttering in the editor
                if (iterator->second.m_atlas && iterator->second.m_atlas->GetTexture())
                {
                    iterator->second.m_atlas->GetTexture().reset();
                }
                AZStd::string imagePath = iterator->second.m_path;
                AZ::Data::Instance<AZ::RPI::Image> texture = LoadAtlasImage(imagePath);
                if (!texture)
                {
                    AZ_Error("TextureAtlasSystemComponent",
                        false,
                        "Failed to find or create an image instance for texture atlas '%s'"
                        "NOTE: File must be in current project or a gem.",
                        imagePath.c_str());
                    TextureAtlas* temp = iterator->second.m_atlas;
                    m_atlases.erase(iterator->first);
                    TextureAtlasNotificationBus::Broadcast(&TextureAtlasNotifications::OnAtlasUnloaded, temp);
                    return;
                }

                iterator->second.m_atlas->SetTexture(texture);
                TextureAtlasNotificationBus::Broadcast(&TextureAtlasNotifications::OnAtlasReloaded, iterator->second.m_atlas);
                break;
            }
        }
    }

    void TextureAtlasSystemComponent::SaveAtlasToFile(
        const AZStd::string& outputPath,
        AtlasCoordinateSets& handles,
        int width, 
        int height)
    {
        TextureAtlasImpl atlas(handles);
        atlas.SetWidth(width);
        atlas.SetHeight(height);
        AZ::Utils::SaveObjectToFile(outputPath, AZ::DataStream::StreamType::ST_XML, &(atlas));
    }

    // Attempts to load an atlas from file
    TextureAtlas* TextureAtlasSystemComponent::LoadAtlas(const AZStd::string& filePath)
    {
        // Dont use an empty string as a path
        if (filePath.empty())
        {
            return nullptr;
        }
        
        // Normalize the file path
        AZStd::string path = filePath;
        AzFramework::ApplicationRequests::Bus::Broadcast(&AzFramework::ApplicationRequests::NormalizePath, path);

        // Check if the atlas is already loaded
        AZStd::string assetPath = path;
        AzFramework::StringFunc::Path::ReplaceExtension(assetPath, "texatlasidx");
        
        auto iterator = m_atlases.find(assetPath);
        if (iterator != m_atlases.end())
        {
            iterator->second.m_refs++;
            return iterator->second.m_atlas;
        }

        // If it isn't loaded, load it
        // Open the file
        AZ::IO::FileIOBase* input = AZ::IO::FileIOBase::GetInstance();
        AZ::IO::HandleType handle;
        input->Open(assetPath.c_str(), AZ::IO::OpenMode::ModeRead, handle);

        // Read the file
        AZ::u64 size;
        input->Size(handle, size);
        char* buffer = new char[size + 1];
        input->Read(handle, buffer, size);
        buffer[size] = 0;

        // Close the file
        input->Close(handle);

        TextureAtlas* loadedAtlas = AZ::Utils::LoadObjectFromBuffer<TextureAtlasImpl>(buffer, size);
        delete[] buffer;
        if (loadedAtlas)
        {
            // Convert to image path based on the atlas path
            AZStd::string imagePath = path;
            AzFramework::StringFunc::Path::ReplaceExtension(imagePath, "texatlas");
            AZ::Data::Instance<AZ::RPI::Image> texture = LoadAtlasImage(imagePath);
            if (!texture)
            {
                AZ_Error("TextureAtlasSystemComponent",
                    false,
                    "Failed to find or create an image instance for texture atlas '%s'"
                    "NOTE: File must be in current project or a gem.",
                    path.c_str());

                delete loadedAtlas;
                return nullptr;
            }
            else
            {
                // Add the atlas to the list
                AtlasInfo info(loadedAtlas, assetPath);
                ++info.m_refs;
                loadedAtlas->SetTexture(texture);
                AZ::Data::AssetCatalogRequestBus::BroadcastResult(info.m_atlasAssetId, &AZ::Data::AssetCatalogRequests::GetAssetIdByPath, assetPath.c_str(),
                    info.m_atlasAssetId.TYPEINFO_Uuid(), false);
                m_atlases[assetPath] = info;
                TextureAtlasNotificationBus::Broadcast(&TextureAtlasNotifications::OnAtlasLoaded, loadedAtlas);
            }
        }
        return loadedAtlas;
    }

    // Lowers the ref count on an atlas. If the ref count it less than one, deletes the atlas
    void TextureAtlasSystemComponent::UnloadAtlas(TextureAtlas* atlas)
    {
        // Check if the atlas is loaded
        for (auto iterator = m_atlases.begin(); iterator != m_atlases.end(); ++iterator)
        {
            if (iterator->second.m_atlas == atlas)
            {
                --iterator->second.m_refs;
                if (iterator->second.m_refs < 1 && iterator->second.m_atlas->GetTexture())
                {
                    AtlasInfo temp = iterator->second;
                    m_atlases.erase(iterator->first);
                    TextureAtlasNotificationBus::Broadcast(&TextureAtlasNotifications::OnAtlasUnloaded, temp.m_atlas);

                    // Tell the renderer to release the texture.
                    if (temp.m_atlas && temp.m_atlas->GetTexture())
                    {
                        temp.m_atlas->GetTexture().reset();
                    }
                    // Delete the atlas
                    SAFE_DELETE(temp.m_atlas);
                }
                return;
            }
        }
    }

    // Searches for an atlas that contains an image
    TextureAtlas* TextureAtlasSystemComponent::FindAtlasContainingImage(const AZStd::string& filePath)
    {
        // Check all atlases
        for (auto iterator = m_atlases.begin(); iterator != m_atlases.end(); ++iterator)
        {
            if (iterator->second.m_atlas->GetAtlasCoordinates(filePath).GetWidth() > 0)
            {
                // If we we found the image return the pointer to the atlas
                return iterator->second.m_atlas;
            }
        }
        return nullptr;
    }
}

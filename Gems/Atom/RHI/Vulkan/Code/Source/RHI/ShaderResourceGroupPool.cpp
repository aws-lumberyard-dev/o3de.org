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
#include "Atom_RHI_Vulkan_precompiled.h"
#include <Atom/RHI/BufferPool.h>
#include <Atom/RHI/BufferView.h>
#include <Atom/RHI/ImageView.h>
#include <RHI/Buffer.h>
#include <RHI/BufferPool.h>
#include <RHI/DescriptorPool.h>
#include <RHI/DescriptorSet.h>
#include <RHI/DescriptorSetLayout.h>
#include <RHI/Device.h>
#include <RHI/ShaderResourceGroup.h>
#include <RHI/ShaderResourceGroupPool.h>

namespace AZ
{
    namespace Vulkan
    {
        RHI::Ptr<ShaderResourceGroupPool> ShaderResourceGroupPool::Create()
        {
            return aznew ShaderResourceGroupPool();
        }

        RHI::ResultCode ShaderResourceGroupPool::InitInternal(RHI::Device& deviceBase, const RHI::ShaderResourceGroupPoolDescriptor& descriptor) 
        {
            auto& device = static_cast<Device&>(deviceBase);
            const RHI::ShaderResourceGroupLayout& layout = *descriptor.m_layout;
            RHI::ResultCode result = RHI::ResultCode::Success;

            m_descriptorSetCount = RHI::Limits::Device::FrameCountMax;

            const uint32_t constantSize = layout.GetConstantDataSize();
            if (constantSize > 0)
            {
                BufferPoolDescriptor bufferPoolDescriptor;
                bufferPoolDescriptor.m_bindFlags = RHI::BufferBindFlags::Constant;
                bufferPoolDescriptor.m_heapMemoryLevel = RHI::HeapMemoryLevel::Host;
                m_constantBufferPool = BufferPool::Create();
                m_constantBufferPool->Init(device, bufferPoolDescriptor);
            }

            DescriptorSetLayout::Descriptor layoutDescriptor;
            layoutDescriptor.m_device = &device;
            layoutDescriptor.m_shaderResouceGroupLayout = &layout;
            m_descriptorSetLayout = device.AcquireDescriptorSetLayout(layoutDescriptor);
            if (!m_descriptorSetLayout)
            {
                return RHI::ResultCode::Fail;
            }

            m_descriptorSetAllocator = RHI::Ptr<DescriptorSetAllocator>(aznew DescriptorSetAllocator);
            // [GFX_TODO] ATOM-679 Set a proper pool size.
            const uint32_t descriptorSetsPerPool = 100;
            DescriptorSetAllocator::Descriptor allocatorDescriptor;
            allocatorDescriptor.m_device = &device;
            allocatorDescriptor.m_layout = m_descriptorSetLayout.get();
            allocatorDescriptor.m_poolSize = m_descriptorSetCount * descriptorSetsPerPool;
            allocatorDescriptor.m_constantDataPool = m_constantBufferPool.get();

            result = m_descriptorSetAllocator->Init(allocatorDescriptor);
            if (result != RHI::ResultCode::Success)
            {
                return result;
            }


            return RHI::ResultCode::Success;
        }

        RHI::ResultCode ShaderResourceGroupPool::InitGroupInternal(RHI::ShaderResourceGroup& groupBase)
        {
            RHI::ResultCode result = RHI::ResultCode::Success;

            auto& group = static_cast<ShaderResourceGroup&>(groupBase);

            for (size_t i = 0; i < m_descriptorSetCount; ++i)
            {
                auto descriptorSet = m_descriptorSetAllocator->Allocate(*m_descriptorSetLayout);
                if (!descriptorSet)
                {
                    return RHI::ResultCode::OutOfMemory;
                }
                group.m_compiledData.push_back(descriptorSet);
            }
            
            return result;
        }

        void ShaderResourceGroupPool::ShutdownInternal()
        {
            auto& device = static_cast<Device&>(GetDevice());
            m_constantBufferPool = nullptr;
            m_currentIteration = 0;
            device.QueueForRelease(m_descriptorSetLayout);
            device.QueueForRelease(m_descriptorSetAllocator);
            Base::ShutdownInternal();
        }

        RHI::ResultCode ShaderResourceGroupPool::CompileGroupInternal(RHI::ShaderResourceGroup& groupBase, const RHI::ShaderResourceGroupData& groupData)
        {
            auto& group = static_cast<ShaderResourceGroup&>(groupBase);
            group.UpdateCompiledDataIndex(m_currentIteration);
            DescriptorSet& descriptorSet = *group.m_compiledData[group.GetCompileDataIndex()];

            const RHI::ShaderResourceGroupLayout* layout = groupData.GetLayout();

            for (uint32_t groupIndex = 0; groupIndex < static_cast<uint32_t>(layout->GetShaderInputListForBuffers().size()); ++groupIndex)
            {
                const RHI::ShaderInputBufferIndex index(groupIndex);
                auto bufViews = groupData.GetBufferViewArray(index);
                uint32_t layoutIndex = m_descriptorSetLayout->GetLayoutIndexFromGroupIndex(groupIndex, DescriptorSetLayout::ResourceType::BufferView);
                descriptorSet.UpdateBufferViews(layoutIndex, bufViews);
            }

            auto const& shaderImageList = layout->GetShaderInputListForImages();
            for (uint32_t groupIndex = 0; groupIndex < static_cast<uint32_t>(layout->GetShaderInputListForImages().size()); ++groupIndex)
            {
                const RHI::ShaderInputImageIndex index(groupIndex);
                auto imgViews = groupData.GetImageViewArray(index);
                uint32_t layoutIndex = m_descriptorSetLayout->GetLayoutIndexFromGroupIndex(groupIndex, DescriptorSetLayout::ResourceType::ImageView);
                descriptorSet.UpdateImageViews(layoutIndex, imgViews, shaderImageList[groupIndex].m_type);
            }

            for (uint32_t groupIndex = 0; groupIndex < static_cast<uint32_t>(layout->GetShaderInputListForBufferUnboundedArrays().size()); ++groupIndex)
            {
                const RHI::ShaderInputBufferUnboundedArrayIndex index(groupIndex);
                auto bufViews = groupData.GetBufferViewUnboundedArray(index);
                uint32_t layoutIndex = m_descriptorSetLayout->GetLayoutIndexFromGroupIndex(groupIndex, DescriptorSetLayout::ResourceType::BufferViewUnboundedArray);
                descriptorSet.UpdateBufferViews(layoutIndex, bufViews);
            }

            auto const& shaderImageUnboundeArrayList = layout->GetShaderInputListForImageUnboundedArrays();
            for (uint32_t groupIndex = 0; groupIndex < static_cast<uint32_t>(layout->GetShaderInputListForImageUnboundedArrays().size()); ++groupIndex)
            {
                const RHI::ShaderInputImageUnboundedArrayIndex index(groupIndex);
                auto imgViews = groupData.GetImageViewUnboundedArray(index);
                uint32_t layoutIndex = m_descriptorSetLayout->GetLayoutIndexFromGroupIndex(groupIndex, DescriptorSetLayout::ResourceType::ImageViewUnboundedArray);
                descriptorSet.UpdateImageViews(layoutIndex, imgViews, shaderImageUnboundeArrayList[groupIndex].m_type);
            }

            for (uint32_t groupIndex = 0; groupIndex < static_cast<uint32_t>(layout->GetShaderInputListForSamplers().size()); ++groupIndex)
            {
                const RHI::ShaderInputSamplerIndex index(groupIndex);
                auto samplerArray = groupData.GetSamplerArray(index);
                uint32_t layoutIndex = m_descriptorSetLayout->GetLayoutIndexFromGroupIndex(groupIndex, DescriptorSetLayout::ResourceType::Sampler);
                descriptorSet.UpdateSamplers(layoutIndex, samplerArray);
            }

            auto constantData = groupData.GetConstantData();
            if (!constantData.empty())
            {
                descriptorSet.UpdateConstantData(constantData);
            }
            descriptorSet.CommitUpdates();

            return RHI::ResultCode::Success;
        }

        void ShaderResourceGroupPool::ShutdownResourceInternal(RHI::Resource& resourceBase)
        {
            ShaderResourceGroup& group = static_cast<ShaderResourceGroup&>(resourceBase);
            for (size_t i = 0; i < m_descriptorSetCount; ++i)
            {
                m_descriptorSetAllocator->DeAllocate(group.m_compiledData[i]);
            }
            Base::ShutdownResourceInternal(resourceBase);
        }

        void ShaderResourceGroupPool::OnFrameEnd()
        {
            m_currentIteration++;
            m_descriptorSetAllocator->Collect();
            Base::OnFrameEnd();
        }
    }
}

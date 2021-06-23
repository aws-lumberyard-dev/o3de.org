/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <LmbrCentral/Dependency/DependencyMonitor.h>
#include <GradientSignal/GradientSampler.h>
#include <AzCore/Component/Component.h>
#include <GradientSignal/Ebuses/GradientRequestBus.h>
#include <GradientSignal/Ebuses/ThresholdGradientRequestBus.h>

namespace LmbrCentral
{
    template<typename, typename>
    class EditorWrappedComponentBase;
}

namespace GradientSignal
{
    class ThresholdGradientConfig
        : public AZ::ComponentConfig
    {
    public:
        AZ_CLASS_ALLOCATOR(ThresholdGradientConfig, AZ::SystemAllocator, 0);
        AZ_RTTI(ThresholdGradientConfig, "{E9E2D5B3-66F1-494D-91D2-1E83D36A1AC1}", AZ::ComponentConfig);
        static void Reflect(AZ::ReflectContext* context);
        GradientSampler m_gradientSampler;
        float m_threshold = 0.5f;
    };

    static const AZ::Uuid ThresholdGradientComponentTypeId = "{CCE70521-E2D8-4304-B748-1E37A6DC57BF}";

    /**
    * calculates a gradient value by converting values from another gradient to 0 or 1
    */      
    class ThresholdGradientComponent
        : public AZ::Component
        , private GradientRequestBus::Handler
        , private ThresholdGradientRequestBus::Handler
    {
    public:
        template<typename, typename> friend class LmbrCentral::EditorWrappedComponentBase;
        AZ_COMPONENT(ThresholdGradientComponent, ThresholdGradientComponentTypeId);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& services);
        static void Reflect(AZ::ReflectContext* context);

        ThresholdGradientComponent(const ThresholdGradientConfig& configuration);
        ThresholdGradientComponent() = default;
        ~ThresholdGradientComponent() = default;

        //////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Activate() override;
        void Deactivate() override;
        bool ReadInConfig(const AZ::ComponentConfig* baseConfig) override;
        bool WriteOutConfig(AZ::ComponentConfig* outBaseConfig) const override;

        //////////////////////////////////////////////////////////////////////////
        // GradientRequestBus
        float GetValue(const GradientSampleParams& sampleParams) const override;
        bool IsEntityInHierarchy(const AZ::EntityId& entityId) const override;

    protected:
        //////////////////////////////////////////////////////////////////////////
        // ThresholdGradientRequestBus
        float GetThreshold() const override;
        void SetThreshold(float threshold) override;

        GradientSampler& GetGradientSampler() override;

    private:
        ThresholdGradientConfig m_configuration;
        LmbrCentral::DependencyMonitor m_dependencyMonitor;
    };
}

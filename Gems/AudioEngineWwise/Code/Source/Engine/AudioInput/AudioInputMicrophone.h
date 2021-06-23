/*
 * Copyright (c) Contributors to the Open 3D Engine Project
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AudioSourceManager.h>

namespace Audio
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    class AudioInputMicrophone
        : public AudioInputSource
    {
    public:
        AudioInputMicrophone(const SAudioInputConfig& sourceConfig);
        ~AudioInputMicrophone() override;

        void ReadInput(const AudioStreamData& data) override;
        void WriteOutput(AkAudioBuffer* akBuffer) override;
        bool IsOk() const override;

        void OnDeactivated() override;
    };

} // namespace Audio

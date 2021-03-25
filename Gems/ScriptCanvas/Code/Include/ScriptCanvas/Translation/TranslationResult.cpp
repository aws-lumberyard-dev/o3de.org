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

#include "TranslationResult.h"

#include <ScriptCanvas/Core/Nodeable.h>
#include <ScriptCanvas/Grammar/AbstractCodeModel.h>

namespace TranslationResultCpp
{
    enum RuntimeInputsVersion
    {
        RemoveGraphType = 0,
        AddedStaticVariables,
        // add your entry above
        Current
    };
}

namespace ScriptCanvas
{
    namespace Translation
    {
        AZStd::sys_time_t SumDurations(const Translations& translation)
        {
            AZStd::sys_time_t result(0);

            for (const auto& t : translation)
            {
                result += t.second.m_duration;
            }

            return result;
        }

        Result::Result(AZStd::string invalidSourceInfo)
            : m_invalidSourceInfo(invalidSourceInfo)
            , m_parseDuration(0)
            , m_translationDuration(0)
        {}

        Result::Result(Grammar::AbstractCodeModelConstPtr model)
            : m_model(model)
            , m_parseDuration(m_model->GetParseDuration())
            , m_translationDuration(0)
        {}
    
        Result::Result(Grammar::AbstractCodeModelConstPtr model, Translations&& translations, Errors&& errors)
            : m_model(model)
            , m_translations(AZStd::move(translations))
            , m_errors(AZStd::move(errors))
            , m_parseDuration(m_model->GetParseDuration())
            , m_translationDuration(SumDurations(m_translations))
        {}

        AZStd::string Result::ErrorsToString() const
        {
            AZStd::string resultString;

            const ValidationResults::ValidationEventList& list = m_model->GetValidationEvents();

            for (auto& entry : list)
            {
                resultString += "* ";
                resultString += entry->GetDescription();
            }

            return resultString;
        }

        bool Result::IsSourceValid() const
        {
            return m_invalidSourceInfo.empty();
        }

        bool Result::IsModelValid() const 
        {
            return m_model->IsErrorFree();
        }

        AZ::Outcome<void, AZStd::string> Result::IsSuccess(TargetFlags /*flag*/) const
        {
            if (!IsSourceValid())
            {
                return AZ::Failure(AZStd::string("Graph translation source was invalid"));
            }
            else if (!IsModelValid())
            {
                return AZ::Failure(AZStd::string::format("Graph conversion to abstract code model failed: %s", ErrorsToString().c_str()));
            }
            else if (!TranslationSucceed(ScriptCanvas::Translation::Lua))
            {
                return AZ::Failure(AZStd::string::format("Graph translation to Lua failed: %s", ErrorsToString().c_str()));
            }
            else
            {
                return AZ::Success();
            }
        }

        bool Result::TranslationSucceed(TargetFlags flag) const
        {
            return m_translations.find(flag) != m_translations.end();
        }

        RuntimeInputs::RuntimeInputs(RuntimeInputs&& rhs)
        {
            *this = AZStd::move(rhs);
        }

        size_t RuntimeInputs::GetParameterSize() const
        {
            return m_nodeables.size() + m_variables.size() + m_entityIds.size();
        }

        RuntimeInputs& RuntimeInputs::operator=(RuntimeInputs&& rhs)
        {
            if (this != &rhs)
            {
                m_execution = AZStd::move(rhs.m_execution);
                m_executionCharacteristics = AZStd::move(rhs.m_executionCharacteristics);
                m_nodeables = AZStd::move(rhs.m_nodeables);
                m_variables = AZStd::move(rhs.m_variables);
                m_entityIds = AZStd::move(rhs.m_entityIds);
                m_staticVariables = AZStd::move(rhs.m_staticVariables);
            }

            return *this;
        }

        void RuntimeInputs::Reflect(AZ::ReflectContext* reflectContext)
        {
            if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext))
            {
                serializeContext->Class<RuntimeInputs>()
                    ->Version(TranslationResultCpp::RuntimeInputsVersion::Current)
                    ->Field("execution", &RuntimeInputs::m_execution)
                    ->Field("executionCharacteristics", &RuntimeInputs::m_executionCharacteristics)
                    ->Field("nodeables", &RuntimeInputs::m_nodeables)
                    ->Field("variables", &RuntimeInputs::m_variables)
                    ->Field("entityIds", &RuntimeInputs::m_entityIds)
                    ->Field("staticVariables", &RuntimeInputs::m_staticVariables)
                    ;
            }
        }

    } 

} 

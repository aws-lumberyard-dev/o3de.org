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

#include <Source/Framework/ScriptCanvasTestVerify.h>

namespace ScriptCanvasTests
{
    ScriptCanvasEditor::UnitTestResult VerifyReporterEditor(const ScriptCanvasEditor::Reporter& reporter)
    {
        ScriptCanvasEditor::UnitTestResult result = ScriptCanvasEditor::UnitTestResult::AssumeFailure();

        AZStd::string details;

        if (!reporter.IsCompiled())
        {
            details = "Graph did not compile.\n";
        }
        else if (reporter.IsReportFinished())
        {
            result.m_compiled = true;

            const auto& successes = reporter.GetSuccess();
            for (const auto& success : successes)
            {
                details += "SUCCESS - ";
                details += success.data();
                details += "\n";
            }

            if (!reporter.IsActivated())
            {
                details += "Graph did not activate\n";
            }

            if (!reporter.IsDeactivated())
            {
                // \todo track this more aggressively, graphs should deactivate
                details += "Graph did not deactivate\n";
            }

            if (!reporter.IsErrorFree())
            {
                details += "Graph had errors\n";
            }

            const auto& failures = reporter.GetFailure();
            for (const auto& failure : failures)
            {
                details += "FAILURE - ";
                details += failure.data();
                details += "\n";
            }

            if (!reporter.IsComplete())
            {
                details += "Graph was not marked complete\n";
            }

            const auto& checkpoints = reporter.GetCheckpoints();
            if (checkpoints.empty())
            {
                details += "No checkpoints or other unit test nodes found, using them can help parse graph test failures\n";
            }
            else
            {
                AZStd::string checkpointPath = "Checkpoint Path:\n";
                int i = 0;

                for (const auto& checkpoint : checkpoints)
                {
                    checkpointPath += AZStd::string::format("%2d: %s\n", ++i, checkpoint.data());
                }
                details += checkpointPath.data();
            }

            if (reporter.IsComplete() && failures.empty())
            {
                result.m_completed = true;
                result.m_consoleOutput = "SUCCEEDED, COMPLETE!\n";
                result.m_consoleOutput += details;
                return result;
            }
        }
        else
        {
            result.m_compiled = true;
            details += "Graph report did not finish\n";
        }

        result.m_consoleOutput = "FAILED!\n";
        result.m_consoleOutput += details;
        return result;
    }

}

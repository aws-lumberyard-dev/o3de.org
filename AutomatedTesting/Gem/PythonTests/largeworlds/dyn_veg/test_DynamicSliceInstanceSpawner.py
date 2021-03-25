"""
All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
its licensors.

For complete copyright and license terms please see the LICENSE at the root of this
distribution (the "License"). All use of this software is governed by the License,
or, if provided, by the license below or the license accompanying this file. Do not
remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
"""

import os
import pytest
import logging

# Bail on the test if ly_test_tools doesn't exist.
pytest.importorskip('ly_test_tools')
import ly_test_tools.environment.file_system as file_system
import automatedtesting_shared.hydra_test_utils as hydra
from ly_remote_console.remote_console_commands import RemoteConsole as RemoteConsole

logger = logging.getLogger(__name__)
test_directory = os.path.join(os.path.dirname(__file__), 'EditorScripts')


@pytest.mark.parametrize('project', ['AutomatedTesting'])
@pytest.mark.parametrize('level', ['tmp_level'])
@pytest.mark.usefixtures("automatic_process_killer")
class TestDynamicSliceInstanceSpawner(object):

    @pytest.fixture
    def remote_console_instance(self, request):
        console = RemoteConsole()

        def teardown():
            if console.connected:
                console.stop()

        request.addfinalizer(teardown)

        return console

    @pytest.mark.test_case_id("C28851763")
    @pytest.mark.SUITE_main
    @pytest.mark.parametrize("launcher_platform", ['windows_editor'])
    def test_DynamicSliceInstanceSpawner_DynamicSliceSpawnerWorks(self, request, editor, level, workspace, project,
                                                                  launcher_platform):
        # Ensure temp level does not already exist
        file_system.delete([os.path.join(workspace.paths.dev(), project, "Levels", level)], True, True)

        cfg_args = [level]

        expected_lines = [
            "DynamicSliceInstanceSpawner:  test started",
            "DynamicSliceInstanceSpawner:  test finished",
            "DynamicSliceInstanceSpawner:  result=SUCCESS"
        ]

        hydra.launch_and_validate_results(request, test_directory, editor,
                                          'DynamicSliceInstanceSpawner_DynamicSliceSpawnerWorks.py',
                                          expected_lines=expected_lines, cfg_args=cfg_args)

        # Cleanup our temp level
        file_system.delete([os.path.join(workspace.paths.dev(), project, "Levels", level)], True, True)

    @pytest.mark.test_case_id('C2574330')
    @pytest.mark.BAT
    @pytest.mark.SUITE_periodic
    @pytest.mark.parametrize("launcher_platform", ['windows_editor'])
    def test_DynamicSliceInstanceSpawner_Embedded_E2E_Editor(self, workspace, request, editor, level, project,
                                                             launcher_platform):
        # Ensure temp level does not already exist
        file_system.delete([os.path.join(workspace.paths.dev(), project, "Levels", level)], True, True)

        expected_lines = [
            "'Instance Spawner' created",
            "'Planting Surface' created",
            "DynamicSliceInstanceSpawnerEmbeddedEditor:  Expected 400 instances - Found 400 instances",
            "DynamicSliceInstanceSpawnerEmbeddedEditor:  result=SUCCESS"
        ]

        hydra.launch_and_validate_results(request, test_directory, editor, "DynamicSliceInstanceSpawner_Embedded_E2E.py",
                                          expected_lines, cfg_args=[level])

    @pytest.mark.test_case_id('C2574330')
    @pytest.mark.BAT
    @pytest.mark.SUITE_periodic
    @pytest.mark.parametrize("launcher_platform", ['windows'])
    def test_DynamicSliceInstanceSpawner_Embedded_E2E_Launcher(self, workspace, launcher, level,
                                                               remote_console_instance, project, launcher_platform):

        expected_lines = [
            "Instances found in area = 400"
        ]

        hydra.launch_and_validate_results_launcher(launcher, level, remote_console_instance, expected_lines)

        # Cleanup our temp level
        file_system.delete([os.path.join(workspace.paths.dev(), project, "Levels", level)], True, True)

    @pytest.mark.test_case_id('C4762367')
    @pytest.mark.SUITE_periodic
    @pytest.mark.parametrize("launcher_platform", ['windows_editor'])
    def test_DynamicSliceInstanceSpawner_External_E2E_Editor(self, workspace, request, editor, level, project,
                                                             launcher_platform):
        # Ensure temp level does not already exist
        file_system.delete([os.path.join(workspace.paths.dev(), project, "Levels", level)], True, True)

        expected_lines = [
            "Spawner entity created",
            "'Planting Surface' created",
            "DynamicSliceInstanceSpawnerExternalEditor:  Expected 400 instances - Found 400 instances",
            "DynamicSliceInstanceSpawnerExternalEditor:  result=SUCCESS"
        ]

        hydra.launch_and_validate_results(request, test_directory, editor, "DynamicSliceInstanceSpawner_External_E2E.py",
                                          expected_lines, cfg_args=[level])

    @pytest.mark.test_case_id('C4762367')
    @pytest.mark.SUITE_periodic
    @pytest.mark.parametrize("launcher_platform", ['windows'])
    def test_DynamicSliceInstanceSpawner_External_E2E_Launcher(self, workspace, launcher, level,
                                                               remote_console_instance, project, launcher_platform):

        expected_lines = [
            "Instances found in area = 400"
        ]

        hydra.launch_and_validate_results_launcher(launcher, level, remote_console_instance, expected_lines)

        # Cleanup our temp level
        file_system.delete([os.path.join(workspace.paths.dev(), project, "Levels", level)], True, True)


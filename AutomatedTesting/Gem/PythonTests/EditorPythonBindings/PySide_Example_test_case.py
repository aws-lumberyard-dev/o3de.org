"""
All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
its licensors.

For complete copyright and license terms please see the LICENSE at the root of this
distribution (the "License"). All use of this software is governed by the License,
or, if provided, by the license below or the license accompanying this file. Do not
remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

"""

#
# This test shows how to use PySide2 inside an Editor Python Bindings test.
# For PySide details see automatedtesting_shared/pyside_utils.py and automatedtesting_shared/pyside_component_utils.py
#

import sys
import os
import PySide2

sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/../automatedtesting_shared')

import azlmbr.bus as bus
import azlmbr.entity as entity
import azlmbr.editor as editor
import azlmbr.legacy.general as general
import pyside_component_utils


def PySide_Example_test_case():
    # Open level, any level should work
    editor.EditorToolsApplicationRequestBus(bus.Broadcast, 'OpenLevelNoPrompt', os.path.join('WhiteBox', 'EmptyLevel'))

    entityId = editor.ToolsApplicationRequestBus(bus.Broadcast, 'CreateNewEntity', entity.EntityId())

    if entityId:
        print('New entity with no parent created')

    # Get Component Type for Environment Probe and attach to entity
    typeIdsList = editor.EditorComponentAPIBus(bus.Broadcast, 'FindComponentTypeIdsByEntityType', ["Environment Probe"],
                                               entity.EntityType().Game)
    componentOutcome = editor.EditorComponentAPIBus(bus.Broadcast, 'AddComponentsOfType', entityId, typeIdsList)
    if componentOutcome.IsSuccess():
        print('Environment Probe component added to entity')

    # Waiting for one frame so that the widgets in the UI are updated with the new component information
    general.idle_enable(True)
    general.idle_wait_frames(1)

    values = pyside_component_utils.get_component_combobox_values('Environment Probe', 'Resolution', print)

    if values:
        print(f'ComboBox Values retrieved: {values}.')
    else:
        print('Could not retrieve ComboBox values')

    editor.EditorToolsApplicationRequestBus(bus.Broadcast, 'ExitNoPrompt')


if __name__ == '__main__':
    PySide_Example_test_case()

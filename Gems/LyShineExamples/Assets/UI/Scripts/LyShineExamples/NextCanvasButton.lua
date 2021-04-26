----------------------------------------------------------------------------------------------------
--
-- All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
-- its licensors.
--
-- For complete copyright and license terms please see the LICENSE at the root of this
-- distribution (the "License"). All use of this software is governed by the License,
-- or, if provided, by the license below or the license accompanying this file. Do not
-- remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--
--
----------------------------------------------------------------------------------------------------

local NextCanvasButton = 
{
    Properties = 
    {
        canvasName = ""
    },
}

function NextCanvasButton:OnActivate()
    self.buttonHandler = UiButtonNotificationBus.Connect(self, self.entityId)
end

function NextCanvasButton:OnDeactivate()
    self.buttonHandler:Disconnect()
end

function NextCanvasButton:OnButtonClick()
    -- Load the next canvas
    UiCanvasManagerBus.Broadcast.LoadCanvas(self.Properties.canvasName)
    -- Unload the current canvas
    canvasId = UiElementBus.Event.GetCanvas(self.entityId)
    if (canvasId:IsValid()) then
        UiCanvasManagerBus.Broadcast.UnloadCanvas(canvasId)
    end
end

return NextCanvasButton

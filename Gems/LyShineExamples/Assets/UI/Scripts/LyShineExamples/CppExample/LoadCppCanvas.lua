----------------------------------------------------------------------------------------------------
--
-- Copyright (c) Contributors to the Open 3D Engine Project
-- 
-- SPDX-License-Identifier: Apache-2.0 OR MIT
--
--
--
----------------------------------------------------------------------------------------------------

local LoadCppCanvas = 
{
    Properties = 
    {
    },
}

function LoadCppCanvas:OnActivate()
    self.buttonHandler = UiButtonNotificationBus.Connect(self, self.entityId)
end

function LoadCppCanvas:OnButtonClick()
    LyShineExamplesCppExampleBus.Broadcast.CreateCanvas()
end

function LoadCppCanvas:OnDeactivate()
    self.buttonHandler:Disconnect()
    LyShineExamplesCppExampleBus.Broadcast.DestroyCanvas()
end

return LoadCppCanvas

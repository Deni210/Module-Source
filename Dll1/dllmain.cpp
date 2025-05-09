#include "InstanceManager/InstanceManager.hpp"
#include "Environment/Environment.hpp"
#include "Execution/Execution.hpp"

#include <filesystem>
#include <thread>
#include "ThreadPool.hpp"

#include <shlobj.h>
#include <psapi.h>
#include <TlHelp32.h>
#include <dbghelp.h>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "dbghelp.lib")

lua_State* exploitThread;
uintptr_t dataModel;

bool Init() {
    dataModel = InstanceManager::GetDataModel();
    if (!dataModel) { // homescreen or datamodel has not yet been initialized
        Sleep(100);
        return Init();
    }

    uintptr_t scriptContext = InstanceManager::GetScriptContext(dataModel);
    if (!scriptContext) { // datamodel has not yet been initialized
        Sleep(100);
        return Init();
    }
    
    *(BYTE*)(scriptContext + 0x6E0) = 1; // require bypass

    uintptr_t globalState = InstanceManager::GetGlobalState(scriptContext + Offsets::GlobalState);
    uintptr_t luaState = InstanceManager::DecryptLuaState(globalState + Offsets::EncryptedState);

    if (!CheckMemory(luaState)) {
        RBX::Print(3, "Invalid LuaState!");
        return false;
    }

    lua_gc((lua_State*)luaState, LUA_GCSTOP, 0);
    exploitThread = lua_newthread((lua_State*)luaState);
    lua_ref((lua_State*)luaState, -1);
    luaL_sandboxthread(exploitThread);
    lua_pop((lua_State*)luaState, 1);
    lua_gc((lua_State*)luaState, LUA_GCRESTART, 0);

    uintptr_t Userdata = (uintptr_t)exploitThread->userdata;
    *(uintptr_t*)(Userdata + Offsets::ExtraSpace::Identity) = 8;
    *(int64_t*)(Userdata + Offsets::ExtraSpace::Capabilities) = ~0ULL;

    Environment::Initialize(exploitThread);

    std::string drawing = R"(local coreGui = gethui()

local camera = workspace.CurrentCamera
local drawingUI = Instance.new("ScreenGui")
drawingUI.Name = ""
drawingUI.IgnoreGuiInset = true
drawingUI.DisplayOrder = 0x7fffffff
drawingUI.Parent = coreGui

local drawingIndex = 0
local drawingFontsEnum = {
	[0] = Font.fromEnum(Enum.Font.Roboto),
	[1] = Font.fromEnum(Enum.Font.Legacy),
	[2] = Font.fromEnum(Enum.Font.SourceSans),
	[3] = Font.fromEnum(Enum.Font.RobotoMono)
}

local function getFontFromIndex(fontIndex)
	return drawingFontsEnum[fontIndex]
end

local function convertTransparency(transparency)
	return math.clamp(1 - transparency, 0, 1)
end

local baseDrawingObj = setmetatable({
	Visible = true,
	ZIndex = 0,
	Transparency = 1,
	Color = Color3.new(),
	Remove = function(self)
		setmetatable(self, nil)
	end,
	Destroy = function(self)
		setmetatable(self, nil)
	end,
	SetProperty = function(self, index, value)
		if self[index] ~= nil then
			self[index] = value
		else
			warn("Attempted to set invalid property: " .. tostring(index))
		end
	end,
	GetProperty = function(self, index)
		if self[index] ~= nil then
			return self[index]
		else
			warn("Attempted to get invalid property: " .. tostring(index))
			return nil
		end
	end,
	SetParent = function(self, parent)
		self.Parent = parent
	end
}, {
	__add = function(t1, t2)
		local result = {}
		for index, value in pairs(t1) do
			result[index] = value
		end
		for index, value in pairs(t2) do
			result[index] = value
		end
		return result
	end
})

local DrawingLib = {}
DrawingLib.Fonts = {
	["UI"] = 0,
	["System"] = 1,
	["Plex"] = 2,
	["Monospace"] = 3
}

function DrawingLib.createLine()
	local lineObj = ({
		From = Vector2.zero,
		To = Vector2.zero,
		Thickness = 1
	} + baseDrawingObj)

	local lineFrame = Instance.new("Frame")
	lineFrame.Name = drawingIndex
	lineFrame.AnchorPoint = Vector2.new(0.5, 0.5)
	lineFrame.BorderSizePixel = 0

	lineFrame.Parent = drawingUI
	return setmetatable({Parent = drawingUI}, {
		__newindex = function(_, index, value)
			if lineObj[index] == nil then 
				warn("Invalid property: " .. tostring(index))
				return 
			end

			if index == "From" or index == "To" then
				local direction = (index == "From" and lineObj.To or value) - (index == "From" and value or lineObj.From)
				local center = (lineObj.To + lineObj.From) / 2
				local distance = direction.Magnitude
				local theta = math.deg(math.atan2(direction.Y, direction.X))

				lineFrame.Position = UDim2.fromOffset(center.X, center.Y)
				lineFrame.Rotation = theta
				lineFrame.Size = UDim2.fromOffset(distance, lineObj.Thickness)
			elseif index == "Thickness" then
				lineFrame.Size = UDim2.fromOffset((lineObj.To - lineObj.From).Magnitude, value)
			elseif index == "Visible" then
				lineFrame.Visible = value
			elseif index == "ZIndex" then
				lineFrame.ZIndex = value
			elseif index == "Transparency" then
				lineFrame.BackgroundTransparency = convertTransparency(value)
			elseif index == "Color" then
				lineFrame.BackgroundColor3 = value
			elseif index == "Parent" then
				lineFrame.Parent = value
			end
			lineObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					lineFrame:Destroy()
					lineObj:Remove()
				end
			end
			return lineObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

function DrawingLib.createText()
	local textObj = ({
		Text = "",
		Font = DrawingLib.Fonts.UI,
		Size = 0,
		Position = Vector2.zero,
		Center = false,
		Outline = false,
		OutlineColor = Color3.new()
	} + baseDrawingObj)

	local textLabel, uiStroke = Instance.new("TextLabel"), Instance.new("UIStroke")
	textLabel.Name = drawingIndex
	textLabel.AnchorPoint = Vector2.new(0.5, 0.5)
	textLabel.BorderSizePixel = 0
	textLabel.BackgroundTransparency = 1

	local function updateTextPosition()
		local textBounds = textLabel.TextBounds
		local offset = textBounds / 2
		textLabel.Size = UDim2.fromOffset(textBounds.X, textBounds.Y)
		textLabel.Position = UDim2.fromOffset(textObj.Position.X + (not textObj.Center and offset.X or 0), textObj.Position.Y + offset.Y)
	end

	textLabel:GetPropertyChangedSignal("TextBounds"):Connect(updateTextPosition)

	uiStroke.Thickness = 1
	uiStroke.Enabled = textObj.Outline
	uiStroke.Color = textObj.Color

	textLabel.Parent, uiStroke.Parent = drawingUI, textLabel

	return setmetatable({Parent = drawingUI}, {
		__newindex = function(_, index, value)
			if textObj[index] == nil then 
				warn("Invalid property: " .. tostring(index))
				return 
			end

			if index == "Text" then
				textLabel.Text = value
			elseif index == "Font" then
				textLabel.FontFace = getFontFromIndex(math.clamp(value, 0, 3))
			elseif index == "Size" then
				textLabel.TextSize = value
			elseif index == "Position" then
				updateTextPosition()
			elseif index == "Center" then
				textLabel.Position = UDim2.fromOffset((value and camera.ViewportSize / 2 or textObj.Position).X, textObj.Position.Y)
			elseif index == "Outline" then
				uiStroke.Enabled = value
			elseif index == "OutlineColor" then
				uiStroke.Color = value
			elseif index == "Visible" then
				textLabel.Visible = value
			elseif index == "ZIndex" then
				textLabel.ZIndex = value
			elseif index == "Transparency" then
				local transparency = convertTransparency(value)
				textLabel.TextTransparency = transparency
				uiStroke.Transparency = transparency
			elseif index == "Color" then
				textLabel.TextColor3 = value
			elseif index == "Parent" then
				textLabel.Parent = value
			end
			textObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					textLabel:Destroy()
					textObj:Remove()
				end
			elseif index == "TextBounds" then
				return textLabel.TextBounds
			end
			return textObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

function DrawingLib.createCircle()
	local circleObj = ({
		Radius = 150,
		Position = Vector2.zero,
		Thickness = 0.7,
		Filled = false
	} + baseDrawingObj)

	local circleFrame, uiCorner, uiStroke = Instance.new("Frame"), Instance.new("UICorner"), Instance.new("UIStroke")
	circleFrame.Name = drawingIndex
	circleFrame.AnchorPoint = Vector2.new(0.5, 0.5)
	circleFrame.BorderSizePixel = 0

	uiCorner.CornerRadius = UDim.new(1, 0)
	circleFrame.Size = UDim2.fromOffset(circleObj.Radius, circleObj.Radius)
	uiStroke.Thickness = circleObj.Thickness
	uiStroke.ApplyStrokeMode = Enum.ApplyStrokeMode.Border

	circleFrame.Parent, uiCorner.Parent, uiStroke.Parent = drawingUI, circleFrame, circleFrame

	return setmetatable({Parent = drawingUI}, {
		__newindex = function(_, index, value)
			if circleObj[index] == nil then 
				warn("Invalid property: " .. tostring(index))
				return 
			end

			if index == "Radius" then
				local radius = value * 2
				circleFrame.Size = UDim2.fromOffset(radius, radius)
			elseif index == "Position" then
				circleFrame.Position = UDim2.fromOffset(value.X, value.Y)
			elseif index == "Thickness" then
				uiStroke.Thickness = math.clamp(value, 0.6, 0x7fffffff)
			elseif index == "Filled" then
				circleFrame.BackgroundTransparency = value and convertTransparency(circleObj.Transparency) or 1
				uiStroke.Enabled = not value
			elseif index == "Visible" then
				circleFrame.Visible = value
			elseif index == "ZIndex" then
				circleFrame.ZIndex = value
			elseif index == "Transparency" then
				local transparency = convertTransparency(value)
				circleFrame.BackgroundTransparency = circleObj.Filled and transparency or 1
				uiStroke.Transparency = transparency
			elseif index == "Color" then
				circleFrame.BackgroundColor3 = value
				uiStroke.Color = value
			elseif index == "Parent" then
				circleFrame.Parent = value
			end
			circleObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					circleFrame:Destroy()
					circleObj:Remove()
				end
			end
			return circleObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

function DrawingLib.createSquare()
	local squareObj = ({
		Size = Vector2.zero,
		Position = Vector2.zero,
		Thickness = 0.7,
		Filled = false
	} + baseDrawingObj)

	local squareFrame, uiStroke = Instance.new("Frame"), Instance.new("UIStroke")
	squareFrame.Name = drawingIndex
	squareFrame.BorderSizePixel = 0

	squareFrame.Parent, uiStroke.Parent = drawingUI, squareFrame

	return setmetatable({Parent = drawingUI}, {
		__newindex = function(_, index, value)
			if squareObj[index] == nil then 
				warn("Invalid property: " .. tostring(index))
				return 
			end

			if index == "Size" then
				squareFrame.Size = UDim2.fromOffset(value.X, value.Y)
			elseif index == "Position" then
				squareFrame.Position = UDim2.fromOffset(value.X, value.Y)
			elseif index == "Thickness" then
				uiStroke.Thickness = math.clamp(value, 0.6, 0x7fffffff)
			elseif index == "Filled" then
				squareFrame.BackgroundTransparency = value and convertTransparency(squareObj.Transparency) or 1
				uiStroke.Enabled = not value
			elseif index == "Visible" then
				squareFrame.Visible = value
			elseif index == "ZIndex" then
				squareFrame.ZIndex = value
			elseif index == "Transparency" then
				local transparency = convertTransparency(value)
				squareFrame.BackgroundTransparency = squareObj.Filled and transparency or 1
				uiStroke.Transparency = transparency
			elseif index == "Color" then
				squareFrame.BackgroundColor3 = value
				uiStroke.Color = value
			elseif index == "Parent" then
				squareFrame.Parent = value
			end
			squareObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					squareFrame:Destroy()
					squareObj:Remove()
				end
			end
			return squareObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

function DrawingLib.createImage()
	local imageObj = ({
		Data = "",
		DataURL = "rbxassetid://0",
		Size = Vector2.zero,
		Position = Vector2.zero
	} + baseDrawingObj)

	local imageFrame = Instance.new("ImageLabel")
	imageFrame.Name = drawingIndex
	imageFrame.BorderSizePixel = 0
	imageFrame.ScaleType = Enum.ScaleType.Stretch
	imageFrame.BackgroundTransparency = 1

	imageFrame.Parent = drawingUI

	return setmetatable({Parent = drawingUI}, {
		__newindex = function(_, index, value)
			if imageObj[index] == nil then 
				warn("Invalid property: " .. tostring(index))
				return 
			end

			if index == "Data" then
			elseif index == "DataURL" then
				imageFrame.Image = value
			elseif index == "Size" then
				imageFrame.Size = UDim2.fromOffset(value.X, value.Y)
			elseif index == "Position" then
				imageFrame.Position = UDim2.fromOffset(value.X, value.Y)
			elseif index == "Visible" then
				imageFrame.Visible = value
			elseif index == "ZIndex" then
				imageFrame.ZIndex = value
			elseif index == "Transparency" then
				imageFrame.ImageTransparency = convertTransparency(value)
			elseif index == "Color" then
				imageFrame.ImageColor3 = value
			elseif index == "Parent" then
				imageFrame.Parent = value
			end
			imageObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					imageFrame:Destroy()
					imageObj:Remove()
				end
			elseif index == "Data" then
				return nil 
			end
			return imageObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

function DrawingLib.createQuad()
	local quadObj = ({
		PointA = Vector2.zero,
		PointB = Vector2.zero,
		PointC = Vector2.zero,
		PointD = Vector2.zero,
		Thickness = 1,
		Filled = false
	} + baseDrawingObj)

	local _linePoints = {
		A = DrawingLib.createLine(),
		B = DrawingLib.createLine(),
		C = DrawingLib.createLine(),
		D = DrawingLib.createLine()
	}

	local fillFrame = Instance.new("Frame")
	fillFrame.Name = drawingIndex .. "_Fill"
	fillFrame.BorderSizePixel = 0
	fillFrame.BackgroundTransparency = quadObj.Transparency
	fillFrame.BackgroundColor3 = quadObj.Color
	fillFrame.ZIndex = quadObj.ZIndex
	fillFrame.Visible = quadObj.Visible and quadObj.Filled

	fillFrame.Parent = drawingUI

	return setmetatable({Parent = drawingUI}, {
		__newindex = function(_, index, value)
			if quadObj[index] == nil then 
				warn("Invalid property: " .. tostring(index))
				return 
			end

			if index == "PointA" then
				_linePoints.A.From = value
				_linePoints.B.To = value
			elseif index == "PointB" then
				_linePoints.B.From = value
				_linePoints.C.To = value
			elseif index == "PointC" then
				_linePoints.C.From = value
				_linePoints.D.To = value
			elseif index == "PointD" then
				_linePoints.D.From = value
				_linePoints.A.To = value
			elseif index == "Thickness" or index == "Visible" or index == "Color" or index == "ZIndex" then
				for _, linePoint in pairs(_linePoints) do
					linePoint[index] = value
				end
				if index == "Visible" then
					fillFrame.Visible = value and quadObj.Filled
				elseif index == "Color" then
					fillFrame.BackgroundColor3 = value
				elseif index == "ZIndex" then
					fillFrame.ZIndex = value
				end
			elseif index == "Filled" then
				for _, linePoint in pairs(_linePoints) do
					linePoint.Transparency = value and 1 or quadObj.Transparency
				end
				fillFrame.Visible = value
			elseif index == "Parent" then
				fillFrame.Parent = value
			end
			quadObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					for _, linePoint in pairs(_linePoints) do
						linePoint:Remove()
					end
					fillFrame:Destroy()
					quadObj:Remove()
				end
			end
			return quadObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

function DrawingLib.createTriangle()
	local triangleObj = ({
		PointA = Vector2.zero,
		PointB = Vector2.zero,
		PointC = Vector2.zero,
		Thickness = 1,
		Filled = false
	} + baseDrawingObj)

	local _linePoints = {
		A = DrawingLib.createLine(),
		B = DrawingLib.createLine(),
		C = DrawingLib.createLine()
	}

	local fillFrame = Instance.new("Frame")
	fillFrame.Name = drawingIndex .. "_Fill"
	fillFrame.BorderSizePixel = 0
	fillFrame.BackgroundTransparency = triangleObj.Transparency
	fillFrame.BackgroundColor3 = triangleObj.Color
	fillFrame.ZIndex = triangleObj.ZIndex
	fillFrame.Visible = triangleObj.Visible and triangleObj.Filled

	fillFrame.Parent = drawingUI

	return setmetatable({Parent = drawingUI}, {
		__newindex = function(_, index, value)
			if triangleObj[index] == nil then 
				warn("Invalid property: " .. tostring(index))
				return 
			end

			if index == "PointA" then
				_linePoints.A.From = value
				_linePoints.B.To = value
			elseif index == "PointB" then
				_linePoints.B.From = value
				_linePoints.C.To = value
			elseif index == "PointC" then
				_linePoints.C.From = value
				_linePoints.A.To = value
			elseif index == "Thickness" or index == "Visible" or index == "Color" or index == "ZIndex" then
				for _, linePoint in pairs(_linePoints) do
					linePoint[index] = value
				end
				if index == "Visible" then
					fillFrame.Visible = value and triangleObj.Filled
				elseif index == "Color" then
					fillFrame.BackgroundColor3 = value
				elseif index == "ZIndex" then
					fillFrame.ZIndex = value
				end
			elseif index == "Filled" then
				for _, linePoint in pairs(_linePoints) do
					linePoint.Transparency = value and 1 or triangleObj.Transparency
				end
				fillFrame.Visible = value
			elseif index == "Parent" then
				fillFrame.Parent = value
			end
			triangleObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					for _, linePoint in pairs(_linePoints) do
						linePoint:Remove()
					end
					fillFrame:Destroy()
					triangleObj:Remove()
				end
			end
			return triangleObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

function DrawingLib.createFrame()
	local frameObj = ({
		Size = UDim2.new(0, 100, 0, 100),
		Position = UDim2.new(0, 0, 0, 0),
		Color = Color3.new(1, 1, 1),
		Transparency = 0,
		Visible = true,
		ZIndex = 1
	} + baseDrawingObj)
	)" R"(
	local frame = Instance.new("Frame")
	frame.Name = drawingIndex
	frame.Size = frameObj.Size
	frame.Position = frameObj.Position
	frame.BackgroundColor3 = frameObj.Color
	frame.BackgroundTransparency = convertTransparency(frameObj.Transparency)
	frame.Visible = frameObj.Visible
	frame.ZIndex = frameObj.ZIndex
	frame.BorderSizePixel = 0

	frame.Parent = drawingUI

	return setmetatable({Parent = drawingUI}, {
		__newindex = function(_, index, value)
			if frameObj[index] == nil then
				warn("Invalid property: " .. tostring(index))
				return
			end

			if index == "Size" then
				frame.Size = value
			elseif index == "Position" then
				frame.Position = value
			elseif index == "Color" then
				frame.BackgroundColor3 = value
			elseif index == "Transparency" then
				frame.BackgroundTransparency = convertTransparency(value)
			elseif index == "Visible" then
				frame.Visible = value
			elseif index == "ZIndex" then
				frame.ZIndex = value
			elseif index == "Parent" then
				frame.Parent = value
			end
			frameObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					frame:Destroy()
					frameObj:Remove()
				end
			end
			return frameObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

function DrawingLib.createScreenGui()
	local screenGuiObj = ({
		IgnoreGuiInset = true,
		DisplayOrder = 0,
		ResetOnSpawn = true,
		ZIndexBehavior = Enum.ZIndexBehavior.Sibling,
		Enabled = true
	} + baseDrawingObj)

	local screenGui = Instance.new("ScreenGui")
	screenGui.Name = drawingIndex
	screenGui.IgnoreGuiInset = screenGuiObj.IgnoreGuiInset
	screenGui.DisplayOrder = screenGuiObj.DisplayOrder
	screenGui.ResetOnSpawn = screenGuiObj.ResetOnSpawn
	screenGui.ZIndexBehavior = screenGuiObj.ZIndexBehavior
	screenGui.Enabled = screenGuiObj.Enabled

	screenGui.Parent = coreGui

	return setmetatable({Parent = coreGui}, {
		__newindex = function(_, index, value)
			if screenGuiObj[index] == nil then
				warn("Invalid property: " .. tostring(index))
				return
			end

			if index == "IgnoreGuiInset" then
				screenGui.IgnoreGuiInset = value
			elseif index == "DisplayOrder" then
				screenGui.DisplayOrder = value
			elseif index == "ResetOnSpawn" then
				screenGui.ResetOnSpawn = value
			elseif index == "ZIndexBehavior" then
				screenGui.ZIndexBehavior = value
			elseif index == "Enabled" then
				screenGui.Enabled = value
			elseif index == "Parent" then
				screenGui.Parent = value
			end
			screenGuiObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					screenGui:Destroy()
					screenGuiObj:Remove()
				end
			end
			return screenGuiObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

function DrawingLib.createTextButton()
	local buttonObj = ({
		Text = "Button",
		Font = DrawingLib.Fonts.UI,
		Size = 20,
		Position = UDim2.new(0, 0, 0, 0),
		Color = Color3.new(1, 1, 1),
		BackgroundColor = Color3.new(0.2, 0.2, 0.2),
		Transparency = 0,
		Visible = true,
		ZIndex = 1,
		MouseButton1Click = nil
	} + baseDrawingObj)

	local button = Instance.new("TextButton")
	button.Name = drawingIndex
	button.Text = buttonObj.Text
	button.FontFace = getFontFromIndex(buttonObj.Font)
	button.TextSize = buttonObj.Size
	button.Position = buttonObj.Position
	button.TextColor3 = buttonObj.Color
	button.BackgroundColor3 = buttonObj.BackgroundColor
	button.BackgroundTransparency = convertTransparency(buttonObj.Transparency)
	button.Visible = buttonObj.Visible
	button.ZIndex = buttonObj.ZIndex

	button.Parent = drawingUI

	local buttonEvents = {}

	return setmetatable({
		Parent = drawingUI,
		Connect = function(_, eventName, callback)
			if eventName == "MouseButton1Click" then
				if buttonEvents["MouseButton1Click"] then
					buttonEvents["MouseButton1Click"]:Disconnect()
				end
				buttonEvents["MouseButton1Click"] = button.MouseButton1Click:Connect(callback)
			else
				warn("Invalid event: " .. tostring(eventName))
			end
		end
	}, {
		__newindex = function(_, index, value)
			if buttonObj[index] == nil then
				warn("Invalid property: " .. tostring(index))
				return
			end

			if index == "Text" then
				button.Text = value
			elseif index == "Font" then
				button.FontFace = getFontFromIndex(math.clamp(value, 0, 3))
			elseif index == "Size" then
				button.TextSize = value
			elseif index == "Position" then
				button.Position = value
			elseif index == "Color" then
				button.TextColor3 = value
			elseif index == "BackgroundColor" then
				button.BackgroundColor3 = value
			elseif index == "Transparency" then
				button.BackgroundTransparency = convertTransparency(value)
			elseif index == "Visible" then
				button.Visible = value
			elseif index == "ZIndex" then
				button.ZIndex = value
			elseif index == "Parent" then
				button.Parent = value
			elseif index == "MouseButton1Click" then
				if typeof(value) == "function" then
					if buttonEvents["MouseButton1Click"] then
						buttonEvents["MouseButton1Click"]:Disconnect()
					end
					buttonEvents["MouseButton1Click"] = button.MouseButton1Click:Connect(value)
				else
					warn("Invalid value for MouseButton1Click: expected function, got " .. typeof(value))
				end
			end
			buttonObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					button:Destroy()
					buttonObj:Remove()
				end
			end
			return buttonObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

function DrawingLib.createTextLabel()
	local labelObj = ({
		Text = "Label",
		Font = DrawingLib.Fonts.UI,
		Size = 20,
		Position = UDim2.new(0, 0, 0, 0),
		Color = Color3.new(1, 1, 1),
		BackgroundColor = Color3.new(0.2, 0.2, 0.2),
		Transparency = 0,
		Visible = true,
		ZIndex = 1
	} + baseDrawingObj)

	local label = Instance.new("TextLabel")
	label.Name = drawingIndex
	label.Text = labelObj.Text
	label.FontFace = getFontFromIndex(labelObj.Font)
	label.TextSize = labelObj.Size
	label.Position = labelObj.Position
	label.TextColor3 = labelObj.Color
	label.BackgroundColor3 = labelObj.BackgroundColor
	label.BackgroundTransparency = convertTransparency(labelObj.Transparency)
	label.Visible = labelObj.Visible
	label.ZIndex = labelObj.ZIndex

	label.Parent = drawingUI

	return setmetatable({Parent = drawingUI}, {
		__newindex = function(_, index, value)
			if labelObj[index] == nil then
				warn("Invalid property: " .. tostring(index))
				return
			end

			if index == "Text" then
				label.Text = value
			elseif index == "Font" then
				label.FontFace = getFontFromIndex(math.clamp(value, 0, 3))
			elseif index == "Size" then
				label.TextSize = value
			elseif index == "Position" then
				label.Position = value
			elseif index == "Color" then
				label.TextColor3 = value
			elseif index == "BackgroundColor" then
				label.BackgroundColor3 = value
			elseif index == "Transparency" then
				label.BackgroundTransparency = convertTransparency(value)
			elseif index == "Visible" then
				label.Visible = value
			elseif index == "ZIndex" then
				label.ZIndex = value
			elseif index == "Parent" then
				label.Parent = value
			end
			labelObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					label:Destroy()
					labelObj:Remove()
				end
			end
			return labelObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

function DrawingLib.createTextBox()
	local boxObj = ({
		Text = "",
		Font = DrawingLib.Fonts.UI,
		Size = 20,
		Position = UDim2.new(0, 0, 0, 0),
		Color = Color3.new(1, 1, 1),
		BackgroundColor = Color3.new(0.2, 0.2, 0.2),
		Transparency = 0,
		Visible = true,
		ZIndex = 1
	} + baseDrawingObj)

	local textBox = Instance.new("TextBox")
	textBox.Name = drawingIndex
	textBox.Text = boxObj.Text
	textBox.FontFace = getFontFromIndex(boxObj.Font)
	textBox.TextSize = boxObj.Size
	textBox.Position = boxObj.Position
	textBox.TextColor3 = boxObj.Color
	textBox.BackgroundColor3 = boxObj.BackgroundColor
	textBox.BackgroundTransparency = convertTransparency(boxObj.Transparency)
	textBox.Visible = boxObj.Visible
	textBox.ZIndex = boxObj.ZIndex

	textBox.Parent = drawingUI

	return setmetatable({Parent = drawingUI}, {
		__newindex = function(_, index, value)
			if boxObj[index] == nil then
				warn("Invalid property: " .. tostring(index))
				return
			end

			if index == "Text" then
				textBox.Text = value
			elseif index == "Font" then
				textBox.FontFace = getFontFromIndex(math.clamp(value, 0, 3))
			elseif index == "Size" then
				textBox.TextSize = value
			elseif index == "Position" then
				textBox.Position = value
			elseif index == "Color" then
				textBox.TextColor3 = value
			elseif index == "BackgroundColor" then
				textBox.BackgroundColor3 = value
			elseif index == "Transparency" then
				textBox.BackgroundTransparency = convertTransparency(value)
			elseif index == "Visible" then
				textBox.Visible = value
			elseif index == "ZIndex" then
				textBox.ZIndex = value
			elseif index == "Parent" then
				textBox.Parent = value
			end
			boxObj[index] = value
		end,
		__index = function(self, index)
			if index == "Remove" or index == "Destroy" then
				return function()
					textBox:Destroy()
					boxObj:Remove()
				end
			end
			return boxObj[index]
		end,
		__tostring = function() return "Drawing" end
	})
end

getgenv().Drawing = {
    Fonts = {
        ["UI"] = 0,
        ["System"] = 1,
        ["Plex"] = 2,
        ["Monospace"] = 3
    },
    
    new = function(drawingType)
        drawingIndex += 1
        if drawingType == "Line" then
            return DrawingLib.createLine()
        elseif drawingType == "Text" then
            return DrawingLib.createText()
        elseif drawingType == "Circle" then
            return DrawingLib.createCircle()
        elseif drawingType == "Square" then
            return DrawingLib.createSquare()
        elseif drawingType == "Image" then
            return DrawingLib.createImage()
        elseif drawingType == "Quad" then
            return DrawingLib.createQuad()
        elseif drawingType == "Triangle" then
            return DrawingLib.createTriangle()
        elseif drawingType == "Frame" then
            return DrawingLib.createFrame()
        elseif drawingType == "ScreenGui" then
            return DrawingLib.createScreenGui()
        elseif drawingType == "TextButton" then
            return DrawingLib.createTextButton()
        elseif drawingType == "TextLabel" then
            return DrawingLib.createTextLabel()
        elseif drawingType == "TextBox" then
            return DrawingLib.createTextBox()
        else
            error("Invalid drawing type: " .. tostring(drawingType))
        end
    end
}

getgenv().isrenderobj = function(drawingObj)
    local success, isrenderobj = pcall(function()
		return drawingObj.Parent == drawingUI
	end)
	if not success then return false end
	return isrenderobj
end

getgenv().getrenderproperty = function(drawingObj, property)
	local success, drawingProperty  = pcall(function()
		return drawingObj[property]
	end)
	if not success then return end

	if drawingProperty ~= nil then
		return drawingProperty
	end
end

getgenv().setrenderproperty = function(drawingObj, property, value)
	assert(getgenv().getrenderproperty(drawingObj, property), "'" .. tostring(property) .. "' is not a valid property of " .. tostring(drawingObj) .. ", " .. tostring(typeof(drawingObj)))
	drawingObj[property]  = value
end

getgenv().cleardrawcache = function()
	for _, drawing in drawingUI:GetDescendants() do
		drawing:Remove()
	end
end

Drawing = getgenv().Drawing
cleardrawcache = getgenv().cleardrawcache
setrenderproperty = getgenv().setrenderproperty
getrenderproperty = getgenv().getrenderproperty
isrenderobj = getgenv().isrenderobj

local function getc(str)
	local sum = 0
	for i = 1, #str do
		sum = (sum + string.byte(str, i)) % 256
	end
	return sum
end
getgenv().crypt.encrypt = newcclosure(function(data, key, iv, mode)
  mode = mode or "CBC"
  iv   = iv   or crypt.generatebytes(16)

  local byteChange = (getc(mode) + getc(iv) + getc(key)) % 256
  local res = {}

  for i = 1, #data do
    res[i] = string.char((string.byte(data, i) + byteChange) % 256)
  end

  return crypt.base64encode(table.concat(res)), iv
end)

getgenv().crypt.decrypt = newcclosure(function(data, key, iv, mode)
  mode = mode or "CBC"

  local decoded    = crypt.base64decode(data)
  local byteChange = (getc(mode) + getc(iv) + getc(key)) % 256
  local res = {}

  for i = 1, #decoded do
    res[i] = string.char((string.byte(decoded, i) - byteChange) % 256)
  end

  return table.concat(res)
end)

crypt.base64encode = base64encode
crypt.base64decode = base64decode
crypt.base64_encode = base64encode
crypt.base64_decode = base64decode
base64 = {encode = base64encode, decode = base64decode}
crypt.base64 = base64
crypt.generatebytes = crypt_generatebytes
crypt.generatekey = crypt_generatekey
crypt.hash = crypt_hash
getgenv().crypt = crypt
)";
				
    Execution::Execute(exploitThread, "setreadonly(debug, false) debug.getregistry = getreg debug.getconstant = getconstant debug.getconstants = getconstants debug.getinfo = getinfo debug.getproto = getproto debug.getprotos = getprotos debug.getstack = getstack debug.getupvalue = getupvalue debug.getupvalues = getupvalues debug.setconstant = setconstant debug.setstack = setstack debug.setupvalue = setupvalue setreadonly(debug, true) cache.invalidate = cache_invalidate cache.iscached = cache_iscached cache.replace = cache_replace http.request = request hookmetamethod = function(tab, method, rep) local mt = getrawmetatable(tab) local old = mt[method] setreadonly(mt, false) mt[method] = rep setreadonly(mt, true) tab = mt return old end WebSocket = { connect = websocket_connect } " + drawing + " cloneref(game:GetService('RunService')).Heartbeat:Connect(HookCaller)");

	SetFunction("execute");

    return true;
}

void Main() {
	std::filesystem::path localAppdata = getenv("LOCALAPPDATA");
	std::filesystem::path library = localAppdata / "RealLibrary";
	std::filesystem::path clogs = library / "Crash Logs";
	if (!std::filesystem::is_directory(library))
		std::filesystem::create_directory(library);

	if (!std::filesystem::is_directory(clogs))
		std::filesystem::create_directory(clogs);

    if(!Init())
        return;

	/*
	while (true) {
		Scheduler::coderun_queue.push_back("print'hey'");
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}*/
	// communication method below
    
    std::filesystem::path indicator = localAppdata / "Injected.txt";
    std::filesystem::path executeFile = localAppdata / "execute.txt";

    FILE* file = fopen(indicator.string().c_str(), "w");
    if (file)
		fclose(file);

	if (!std::filesystem::exists(executeFile)) {
		FILE* f = fopen(executeFile.string().c_str(), "w");
		if (f)
			fclose(f);
	}

    std::filesystem::file_time_type lastFileTime = std::filesystem::last_write_time(executeFile);

    while (true) {
        std::filesystem::file_time_type ftime = std::filesystem::last_write_time(executeFile);
        if (ftime != lastFileTime) {
            lastFileTime = ftime;

            uintptr_t newDataModel = InstanceManager::GetDataModel();
            if (newDataModel && newDataModel != dataModel) {
                dataModel = newDataModel;
                if (!Init()) return;
            }
            else if(!newDataModel) continue; // homescreen or datamodel has not yet been initialized

            FILE* file = fopen(executeFile.string().c_str(), "rb");
            if (!file) continue;

            fseek(file, 0, SEEK_END);
            size_t fileSize = ftell(file);
            rewind(file);

            std::string script(fileSize, '\0');
            size_t bytesread = fread(&script[0], 1, fileSize, file);
            fclose(file);

            if (bytesread <= 2) continue;

            Scheduler::coderun_queue.push_back(script);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

std::string getCurrentTimeString() {
    std::time_t rawTime;
    struct tm timeInfo;
    std::time(&rawTime);
    localtime_s(&timeInfo, &rawTime);
    char timeBuffer[19];
    std::sprintf(timeBuffer, "%02d-%02d_%02d-%02d-%02d",
        timeInfo.tm_mday,
        timeInfo.tm_mon + 1,
        timeInfo.tm_hour,
        timeInfo.tm_min,
        timeInfo.tm_sec);
    return std::string(timeBuffer);
}

void SuspendAllThreads() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE) return;

    DWORD processID = GetCurrentProcessId();
    DWORD currentThread = GetCurrentThreadId();
    THREADENTRY32 te;
    te.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(snap, &te)) {
        do {
            if (te.th32OwnerProcessID == processID) {
                HANDLE thread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                if (thread) {
                    SuspendThread(thread);
                    CloseHandle(thread);
                }
            }
        } while (Thread32Next(snap, &te));
    }
    CloseHandle(snap);
}

std::string CrashLogs;

LONG __stdcall exceptionFilter(EXCEPTION_POINTERS* exceptionInfo) {
    EXCEPTION_RECORD* excRecord = exceptionInfo->ExceptionRecord;
    CONTEXT* context = exceptionInfo->ContextRecord;

    if (excRecord->ExceptionAddress == 0) return EXCEPTION_CONTINUE_SEARCH;
    if (excRecord->ExceptionCode == 0x40010006 || excRecord->ExceptionCode == 0x4001000A || excRecord->ExceptionCode == 0x406D1388 || excRecord->ExceptionCode == 0xE06D7363 || excRecord->ExceptionCode == 0x80000003 || excRecord->ExceptionCode == 0xC000001D || excRecord->ExceptionCode == 0x80000004)
        return EXCEPTION_CONTINUE_SEARCH;

    std::string currtime = getCurrentTimeString();
    std::string logFilePath = CrashLogs + "\\crash_" + currtime + ".log";

    FILE* file = fopen(logFilePath.c_str(), "w");
    if (!file) return -1;

    fprintf(file, "Exception Code: 0x%08X\n", excRecord->ExceptionCode);
    fprintf(file, "Exception Address: 0x%p\n", excRecord->ExceptionAddress);

    const char* exceptionType;
    switch (excRecord->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION: exceptionType = "Access Violation"; break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: exceptionType = "Array Bounds Exceeded"; break;
    case EXCEPTION_BREAKPOINT: exceptionType = "Breakpoint"; break;
    case EXCEPTION_DATATYPE_MISALIGNMENT: exceptionType = "Datatype Misalignment"; break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO: exceptionType = "Float Divide by Zero"; break;
    case EXCEPTION_FLT_OVERFLOW: exceptionType = "Float Overflow"; break;
    case EXCEPTION_FLT_UNDERFLOW: exceptionType = "Float Underflow"; break;
    case EXCEPTION_ILLEGAL_INSTRUCTION: exceptionType = "Illegal Instruction"; break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO: exceptionType = "Integer Divide by Zero"; break;
    case EXCEPTION_PRIV_INSTRUCTION: exceptionType = "Privileged Instruction"; break;
    case EXCEPTION_STACK_OVERFLOW: exceptionType = "Stack Overflow"; break;
    case 0xE06D7363: exceptionType = "C++ Exception (0xE06D7363)"; break;
    default: exceptionType = "Unknown Exception"; break;
    }

    fprintf(file, "Exception Type: %s\n", exceptionType);

    if (excRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && excRecord->NumberParameters >= 2) {
        const char* accessType = excRecord->ExceptionInformation[0] ? "write" : "read";
        fprintf(file, "Access Violation Type: %s\n", accessType);
        fprintf(file, "Access Address: 0x%p\n", (void*)excRecord->ExceptionInformation[1]);
    }

    fprintf(file, "\n--- Registers ---\n");
    fprintf(file, "RAX: 0x%016llX\n", context->Rax);
    fprintf(file, "RBX: 0x%016llX\n", context->Rbx);
    fprintf(file, "RCX: 0x%016llX\n", context->Rcx);
    fprintf(file, "RDX: 0x%016llX\n", context->Rdx);
    fprintf(file, "RSI: 0x%016llX\n", context->Rsi);
    fprintf(file, "RDI: 0x%016llX\n", context->Rdi);
    fprintf(file, "RSP: 0x%016llX\n", context->Rsp);
    fprintf(file, "RBP: 0x%016llX\n", context->Rbp);
    fprintf(file, "RIP: 0x%016llX\n", context->Rip);

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    STACKFRAME64 stackFrame = {};
    DWORD machineType = IMAGE_FILE_MACHINE_AMD64;

    stackFrame.AddrPC.Offset = context->Rip;
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = context->Rbp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = context->Rsp;
    stackFrame.AddrStack.Mode = AddrModeFlat;

    fprintf(file, "\n--- Call Stack ---\n");

    fprintf(file, GetFunction().data());
    fprintf(file, "\n");

    while (StackWalk64(machineType, process, GetCurrentThread(), &stackFrame, context, NULL,
        SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
        if (stackFrame.AddrPC.Offset == 0) break;

        void* addr = (void*)stackFrame.AddrPC.Offset;
        fprintf(file, "0x%p\n", addr);

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        if (SymFromAddr(process, (DWORD64)addr, NULL, symbol)) {
            fprintf(file, "    Function: %s\n", symbol->Name);
        }
        else {
            fprintf(file, "    Function: Unknown\n");
        }

        IMAGEHLP_LINE64 line = {};
        DWORD displacement = 0;
        line.SizeOfStruct = sizeof(line);
        if (SymGetLineFromAddr64(process, (DWORD64)addr, &displacement, &line)) {
            fprintf(file, "    File: %s:%lu\n", line.FileName, line.LineNumber);
        }

        DWORD64 moduleBase = SymGetModuleBase64(process, (DWORD64)addr);
        if (moduleBase) {
            char moduleName[MAX_PATH] = {};
            if (GetModuleFileNameA((HMODULE)moduleBase, moduleName, MAX_PATH)) {
                fprintf(file, "    Module: %s\n", moduleName);
            }
        }
    }

    fclose(file);

    std::thread(SuspendAllThreads).detach();
    MessageBox(NULL, "Roblox crashed.", "Crash", MB_OK);
    return EXCEPTION_CONTINUE_SEARCH;
}

int __stdcall DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == 1) {
        std::string localAppdata = getenv("LOCALAPPDATA");
        CrashLogs = localAppdata + "\\RealLibrary\\Crash Logs";
		// uncomment these 3 lines to enable crash logging

		//AddVectoredContinueHandler(1, exceptionFilter);
        //AddVectoredExceptionHandler(1, exceptionFilter);
        //SetUnhandledExceptionFilter(exceptionFilter);

		// if you can't figure out which function crashes, ctrl + click SetFunction (go to definition) and enable debugging
		SetFunction("DLLMain");

		std::thread(Main).detach();
    }

    return 1;
}
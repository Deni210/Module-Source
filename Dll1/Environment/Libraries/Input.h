#pragma once

#include <lstate.h>
#include <lualib.h>
#include <lapi.h>

namespace Input {
	int isrbxactive(lua_State* L) {
        SetFunction("input::isrbxactive");
        HWND foregroundWindow = GetForegroundWindow();
        if (!foregroundWindow) {
            lua_pushboolean(L, false);

            return 1;
        }

        DWORD foregroundProcessID;
        GetWindowThreadProcessId(foregroundWindow, &foregroundProcessID);

        DWORD currentProcessID = GetCurrentProcessId();
        lua_pushboolean(L, foregroundProcessID == currentProcessID);

        return 1;
	}

    int mouse1click(lua_State* L) {
        SetFunction("input::mouse1click");
        HWND window = FindWindow(NULL, "Roblox");
        if (GetForegroundWindow() == window)
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);

        return 0;
    }

    int mouse1press(lua_State* L) {
        SetFunction("input::mouse1press");
        HWND window = FindWindow(NULL, "Roblox");
        if (GetForegroundWindow() == window)
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);

        return 0;
    }

    int mouse1release(lua_State* L) {
        SetFunction("input::mouse1release");
        HWND window = FindWindow(NULL, "Roblox");
        if (GetForegroundWindow() == window)
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

        return 0;
    }

    int mouse2click(lua_State* L) {
        SetFunction("input::mouse2click");
        HWND window = FindWindow(NULL, "Roblox");
        if (GetForegroundWindow() == window)
            mouse_event(MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);

        return 0;
    }

    int mouse2press(lua_State* L) {
        SetFunction("input::mouse2press");
        HWND window = FindWindow(NULL, "Roblox");
        if (GetForegroundWindow() == window)
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);

        return 0;
    }

    int mouse2release(lua_State* L) {
        SetFunction("input::mouse2release");
        HWND window = FindWindow(NULL, "Roblox");
        if (GetForegroundWindow() == window)
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);

        return 0;
    }

    int mousemoveabs(lua_State* L) {
        SetFunction("input::mousemoveabs");
        luaL_checktype(L, 1, LUA_TNUMBER);
        luaL_checktype(L, 2, LUA_TNUMBER);

        HWND window = FindWindow(NULL, "Roblox");

        DWORD x = lua_tonumber(L, 0);
        DWORD y = lua_tonumber(L, 2);

        if (GetForegroundWindow() != window) return 0;

        int width = GetSystemMetrics(SM_CXSCREEN) - 1;
        int height = GetSystemMetrics(SM_CYSCREEN) - 1;

        RECT CRect;
        GetClientRect(GetForegroundWindow(), &CRect);

        POINT Point{ CRect.left, CRect.top };
        ClientToScreen(GetForegroundWindow(), &Point);

        x = (x + (DWORD)Point.x) * (65535 / width);
        y = (y + (DWORD)Point.y) * (65535 / height);

        mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, x, y, 0, 0);

        return 0;
    }

    int mousemoverel(lua_State* L) {
        SetFunction("input::mousemoverel");
        luaL_checktype(L, 1, LUA_TNUMBER);
        luaL_checktype(L, 2, LUA_TNUMBER);

        HWND window = FindWindow(NULL, "Roblox");

        DWORD x = lua_tonumber(L, 0);
        DWORD y = lua_tonumber(L, 2);

        if (GetForegroundWindow() == window)
            mouse_event(MOUSEEVENTF_MOVE, x, y, 0, 0);

        return 0;
    }

    int mousescroll(lua_State* L) {
        SetFunction("input::mousescroll");
        luaL_checktype(L, 1, LUA_TNUMBER);

        HWND window = FindWindow(NULL, "Roblox");
        DWORD scroll_amount = lua_tonumber(L, 1);

        if (GetForegroundWindow() == window)
            mouse_event(MOUSEEVENTF_WHEEL, 0, 0, scroll_amount, 0);

        return 0;
    }

    int iskeydown(lua_State* L) {
        SetFunction("input::iskeydown");
        luaL_checktype(L, 1, LUA_TNUMBER);

        UINT key = lua_tonumber(L, 1);

        SHORT state = GetAsyncKeyState(key);
        lua_pushboolean(L, (state & 0x8000) != 0 ? 1 : 0);

        return 1;
    }

    int keypress(lua_State* L) {
        SetFunction("input::keypress");
        luaL_checktype(L, 1, LUA_TNUMBER);

        HWND window = FindWindow(NULL, "Roblox");
        UINT key = lua_tonumber(L, 1);

        if (GetForegroundWindow() == window)
            keybd_event(0, (BYTE)MapVirtualKey(key, MAPVK_VK_TO_VSC), KEYEVENTF_SCANCODE, 0);
        return 0;
    }

    int keyrelease(lua_State* L) {
        SetFunction("input::keyrelease");
        luaL_checktype(L, 1, LUA_TNUMBER);

        HWND window = FindWindow(NULL, "Roblox");
        UINT key = lua_tonumber(L, 1);

        if (GetForegroundWindow() == window)
            keybd_event(0, (BYTE)MapVirtualKey(key, MAPVK_VK_TO_VSC), KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP, 0);
        return 0;
    }
}
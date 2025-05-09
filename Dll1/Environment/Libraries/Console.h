#pragma once

#include <lstate.h>
#include <lualib.h>
#include <lapi.h>
#include <thread>

bool ExistsConsole = false;
HANDLE OCH = INVALID_HANDLE_VALUE;

void BypassConsole(bool show) {
    PVOID peb = (PVOID)__readgsqword(0x60);
    if (!peb) return;

    PVOID processParameters = *(PVOID*)((BYTE*)peb + 0x20);
    if (!processParameters) return;

    HANDLE* consoleHandle = (HANDLE*)((BYTE*)processParameters + 0x10);
    if (!show) {
        if (OCH == INVALID_HANDLE_VALUE) {
            OCH = *consoleHandle;
        }
        *consoleHandle = NULL;
    }
    else {
        *consoleHandle = OCH;
    }
}

void DelayBypass() {
    Sleep(100);
    BypassConsole(false);
}

std::string GetInput() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE) {
        MessageBoxA(0, "Could not get handle!", "Real Executor", 0);
        return "";
    }

    char buffer[256] = { 0 };
    DWORD bytesRead;

    BypassConsole(true);
    std::thread(DelayBypass).detach();
    if (ReadConsole(hInput, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        buffer[bytesRead - 2] = '\0';
        return std::string(buffer);
    }
    else {
        MessageBoxA(0, "Error reading console!", "Real Executor", 0);
        return "";
    }

    return "";
}

void ClearConsole() {
    BypassConsole(true);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    COORD homeCoords = { 0, 0 };

    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        FillConsoleOutputCharacter(hConsole, ' ', csbi.dwSize.X * csbi.dwSize.Y, homeCoords, &count);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, csbi.dwSize.X * csbi.dwSize.Y, homeCoords, &count);
        SetConsoleCursorPosition(hConsole, homeCoords);
    }
    BypassConsole(false);
}

namespace ConsoleLib {
    int rconsolecreate(lua_State* L) {
        if (ExistsConsole == true) {
            BypassConsole(true);
            ShowWindow(GetConsoleWindow(), SW_SHOW);
            BypassConsole(false);
            return true;
        }
        if (!AllocConsole()) {
            MessageBoxA(NULL, "Error Creating Console", "Error", MB_ICONERROR);
            return false;
        }

        FILE* f;
        if (freopen_s(&f, "CONOUT$", "w", stdout) != 0) {
            MessageBoxA(NULL, "Console Setup Error (1)", "Error", MB_ICONERROR);
        }
        if (freopen_s(&f, "CONOUT$", "w", stderr) != 0) {
            MessageBoxA(NULL, "Console Setup Error (2)", "Error", MB_ICONERROR);
        }
        if (freopen_s(&f, "CONOUT$", "r", stdin) != 0) {
            MessageBoxA(NULL, "Console Setup Error (3)", "Error", MB_ICONERROR);
        }

        SetConsoleTitle("Roblox");
        BypassConsole(false);

        DWORD mode;
        BOOL result = GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode);

        if (!result) {
            printf("Console Loaded Successfully");
            ExistsConsole = true;
        }
        else {
            MessageBox(0, "Your about to get Cooked", "Your about to get Cooked", 0);
        }

        return 0;
    }

    int rconsoleclear(lua_State* L) {
        ClearConsole();

        return 0;
    }

    int rconsoleinput(lua_State* L) {
        std::string result = GetInput();
        lua_pushlstring(L, result.data(), result.size());

        return 1;
    }

    int rconsoleprint(lua_State* L) {
        luaL_checktype(L, 1, LUA_TSTRING);
        
        std::string toprint = lua_tostring(L, 1);

        printf(toprint.data());

        return 0;
    }

    int rconsoledestroy(lua_State* L) {
        if (ExistsConsole) {
            ClearConsole();
            BypassConsole(true);
            SetConsoleTitle("Roblox");
            ShowWindow(GetConsoleWindow(), SW_HIDE);
            BypassConsole(false);
        }

        return 0;
    }

    int rconsolesettitle(lua_State* L) {
        luaL_checktype(L, 1, LUA_TSTRING);
        
        std::string title = lua_tostring(L, 1);

        if (title.data()) {
            BypassConsole(true);
            SetConsoleTitle(title.data());
            BypassConsole(false);
        }

        return 0;
    }

    int rconsoletopmost(lua_State* L) {
        if (ExistsConsole) {
            BypassConsole(true);
            SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            BypassConsole(false);
        }

        return 0;
    }

    int rconsoleinfo(lua_State* L) {
        luaL_checktype(L, 1, LUA_TSTRING);

        std::string toprint = lua_tostring(L, 1);

        BypassConsole(true);
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);

        printf("%s\n", toprint.data());

        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        BypassConsole(false);

        return 0;
    }

    int rconsolewarn(lua_State* L) {
        luaL_checktype(L, 1, LUA_TSTRING);

        std::string toprint = lua_tostring(L, 1);

        BypassConsole(true);
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

        printf("%s\n", toprint.data());

        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        BypassConsole(false);

        return 0;
    }

    int rconsoleerr(lua_State* L) {
        luaL_checktype(L, 1, LUA_TSTRING);

        std::string toprint = lua_tostring(L, 1);

        BypassConsole(true);
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);

        printf("%s\n", toprint.data());

        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        BypassConsole(false);

        return 0;
    }

    int rconsolehide(lua_State* L) {
        BypassConsole(true);
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        BypassConsole(false);

        return 0;
    }

    int rconsoleshow(lua_State* L) {
        BypassConsole(true);
        ShowWindow(GetConsoleWindow(), SW_SHOW);
        BypassConsole(false);

        return 0;
    }

    int rconsoletoggle(lua_State* L) {
        BypassConsole(true);

        HWND consoleWnd = GetConsoleWindow();
        if (consoleWnd) {
            if (IsWindowVisible(consoleWnd)) {
                ShowWindow(consoleWnd, SW_HIDE);
            }
            else {
                ShowWindow(consoleWnd, SW_SHOW);
            }
        }

        BypassConsole(false);
        return 0;
    }

    int rconsolehidden(lua_State* L) {
        BypassConsole(true);

        HWND consoleWnd = GetConsoleWindow();
        if (consoleWnd) {
            if (IsWindowVisible(consoleWnd)) {
                lua_pushboolean(L, 0);
            }
            else {
                lua_pushboolean(L, 1);
            }
        }
        else {
            lua_pushboolean(L, 0);
        }
        BypassConsole(false);

        return 1;
    }
}
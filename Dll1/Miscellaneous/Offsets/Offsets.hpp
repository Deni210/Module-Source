// version-ff05edc617954c5b

#pragma once

#include <Windows.h>
#include <vector>

struct lua_State;

inline const uintptr_t Base = (uintptr_t)GetModuleHandle(nullptr);
#define REBASE(x) (Base + (x))

namespace Offsets {
	const uintptr_t LuaO_NilObject = REBASE(0x47AAE88);
	const uintptr_t LuaH_DummyNode = REBASE(0x47AA5B8);
	const uintptr_t Luau_Execute = REBASE(0x27F5E40);

	const uintptr_t Print = REBASE(0x16C1A30);
	const uintptr_t GetProperty = REBASE(0xBD0540);
	const uintptr_t GetGlobalState = REBASE(0xF2A5F0);
	const uintptr_t DecryptLuaState = REBASE(0xCBDE70);
	const uintptr_t PushInstance = REBASE(0x1001240);
	const uintptr_t LuaVM__Load = REBASE(0xCC0A60);
	const uintptr_t Task__Defer = REBASE(0x115C9C0);
	const uintptr_t FireMouseClick = REBASE(0x1DC0D30);
	const uintptr_t FireRightMouseClick = REBASE(0x1DC0ED0);
	const uintptr_t FireMouseHoverEnter = REBASE(0x1DC22D0);
	const uintptr_t FireMouseHoverLeave = REBASE(0x1DC2470);
	const uintptr_t FireTouchInterest = REBASE(0x15ACC90);
	const uintptr_t FireProximityPrompt = REBASE(0x1E92B00);
	const uintptr_t RequestCode = REBASE(0xA82310);
	const uintptr_t GetCurrentThreadId = REBASE(0x3876E00);
	const uintptr_t GetIdentityStruct = REBASE(0x3877030);
	const uintptr_t IdentityPtr = REBASE(0x654CB68);
	const uintptr_t RaiseEventInvocation = REBASE(0x16743C0);
	const uintptr_t LockViolationInstanceCrash = REBASE(0x60FBEF8);
	const uintptr_t GetValues = REBASE(0xD4D300);

	const uintptr_t LuaD_throw = REBASE(0x27C35E0);

	const uintptr_t RawScheduler = REBASE(0x69CD068);
	const uintptr_t KTable = REBASE(0x64EE730);

	const uintptr_t BitMap = (uintptr_t)GetModuleHandle("RobloxPlayerBeta.dll") + 0x283AC0;

	const uintptr_t GlobalState = 0x140;
	const uintptr_t EncryptedState = 0x88;

	namespace DataModel {
		const uintptr_t FakeDataModel = REBASE(0x690D168);
		const uintptr_t FakeDataModelToDataModel = 0x1B8;

		const uintptr_t ScriptContext = 0x3B0;
	}

	namespace Instance {
		const uintptr_t ClassDescriptor = 0x18;
		const uintptr_t PropertyDescriptor = 0x3B8;
		const uintptr_t ClassName = 0x8;
		const uintptr_t Name = 0x78;
		const uintptr_t Children = 0x80;
	}

	namespace Scripts {
		const uintptr_t LocalScriptEmbedded = 0x1B0;
		const uintptr_t ModuleScriptEmbedded = 0x158;

		const uintptr_t RunContext = 0x150;

		const uintptr_t weak_thread_node = 0x188;
		const uintptr_t weak_thread_ref = 0x8;
		const uintptr_t weak_thread_ref_live = 0x20;
		const uintptr_t weak_thread_ref_live_thread = 0x8;
	}

	namespace ExtraSpace {
		const uintptr_t Identity = 0x30;
		const uintptr_t Capabilities = 0x48;
	}

	namespace TaskScheduler {
		const uintptr_t FpsCap = 0x1B0;
	}
}

namespace RBX {
	using TPrint = void(__fastcall*)(int, const char*);
	inline auto Print = (TPrint)Offsets::Print;

	using TLuaVM__Load = int(__fastcall*)(lua_State*, void*, const char*, int);
	inline auto LuaVM__Load = (TLuaVM__Load)Offsets::LuaVM__Load;

	using TTask__Defer = int(__fastcall*)(lua_State*);
	inline auto Task__Defer = (TTask__Defer)Offsets::Task__Defer;

	using TGetGlobalState = uintptr_t(__fastcall*)(uintptr_t, int32_t*, uintptr_t*);
	inline auto GetGlobalState = (TGetGlobalState)Offsets::GetGlobalState;

	using TDecryptLuaState = uintptr_t(__fastcall*)(uintptr_t);
	inline auto DecryptLuaState = (TDecryptLuaState)Offsets::DecryptLuaState;

	using TPushInstance = void(__fastcall*)(lua_State* state, void* instance);
	inline auto PushInstance = (TPushInstance)Offsets::PushInstance;

	using TLuaD_throw = void(__fastcall*)(lua_State*, int);
	inline auto LuaD_throw = (TLuaD_throw)Offsets::LuaD_throw;

	using TGetProperty = uintptr_t*(__thiscall*)(uintptr_t, uintptr_t*);
	inline auto GetProperty = (TGetProperty)Offsets::GetProperty;

	using TFireMouseClick = void(__fastcall*)(__int64 a1, float a2, __int64 a3);
	inline auto FireMouseClick = (TFireMouseClick)Offsets::FireMouseClick;

	using TFireRightMouseClick = void(__fastcall*)(__int64 a1, float a2, __int64 a3);
	inline auto FireRightMouseClick = (TFireRightMouseClick)Offsets::FireRightMouseClick;

	using TFireMouseHoverEnter = void(__fastcall*)(__int64 a1, __int64 a2);
	inline auto FireMouseHoverEnter = (TFireMouseHoverEnter)Offsets::FireMouseHoverEnter;

	using TFireMouseHoverLeave = void(__fastcall*)(__int64 a1, __int64 a2);
	inline auto FireMouseHoverLeave = (TFireMouseHoverLeave)Offsets::FireMouseHoverLeave;

	using TFireTouchInterest = void(__fastcall*)(uintptr_t, uintptr_t, uintptr_t, bool, bool);
	inline auto FireTouchInterest = (TFireTouchInterest)Offsets::FireTouchInterest;

	using TFireProxmityPrompt = std::uintptr_t(__fastcall*)(std::uintptr_t prompt);
	inline auto FireProximityPrompt = (TFireProxmityPrompt)Offsets::FireProximityPrompt;

	using TRequestCode = uintptr_t(__fastcall*)(uintptr_t protected_string_ref, uintptr_t script);
	inline auto RequestCode = (TRequestCode)Offsets::RequestCode;

	using TGetIdentityStruct = uintptr_t(__fastcall*)(uintptr_t);
	inline auto GetIdentityStruct = (TGetIdentityStruct)Offsets::GetIdentityStruct;

	inline std::string RequestBytecode(uintptr_t scriptPtr) {
		uintptr_t code[0x4];
		std::memset(code, 0, sizeof(code));

		RequestCode((std::uintptr_t)code, scriptPtr);

		std::uintptr_t bytecodePtr = code[1];

		if (!bytecodePtr) { return "Failed to get bytecode"; }

		std::uintptr_t str = bytecodePtr + 0x10;
		std::uintptr_t data;

		if (*reinterpret_cast<std::size_t*>(str + 0x18) > 0xf) {
			data = *reinterpret_cast<std::uintptr_t*>(str);
		}
		else {
			data = str;
		}

		std::string BOOOHOOOOOOOO;
		std::size_t len = *reinterpret_cast<std::size_t*>(str + 0x10);
		BOOOHOOOOOOOO.reserve(len);

		for (unsigned i = 0; i < len; i++) {
			BOOOHOOOOOOOO += *reinterpret_cast<char*>(data + i);
		}

		if (BOOOHOOOOOOOO.size() <= 8) { "Failed to get bytecode"; }

		return BOOOHOOOOOOOO;
	}
}
#pragma once

#include <Windows.h>
#include <string>

#include "Offsets/Offsets.hpp"
#include "Tools.hpp"

namespace InstanceManager {
	uintptr_t GetDataModel() {
		uintptr_t fakeDataModel = *(uintptr_t*)Offsets::DataModel::FakeDataModel;
		if (!CheckMemory(fakeDataModel))
			return 0x0;

		uintptr_t dataModel = *(uintptr_t*)(fakeDataModel + Offsets::DataModel::FakeDataModelToDataModel);
		if (!CheckMemory(dataModel))
			return 0x0;

		uintptr_t namePointer = *(uintptr_t*)(dataModel + Offsets::Instance::Name);
		if (!CheckMemory(namePointer))
			return 0x0;

		std::string dataModelName = *(std::string*)namePointer;
		if (dataModelName == "LuaApp")
			return 0x0;

		return dataModel;
	}

	uintptr_t GetScriptContext(uintptr_t dataModel) {
		uintptr_t childrenContainer = *(uintptr_t*)(dataModel + Offsets::Instance::Children);
		if (!CheckMemory(childrenContainer))
			return 0x0;

		uintptr_t children = *(uintptr_t*)childrenContainer;
		if (!CheckMemory(children))
			return 0x0;

		uintptr_t scriptContext = *(uintptr_t*)(children + Offsets::DataModel::ScriptContext);
		if (!CheckMemory(scriptContext))
			return 0x0;

		return scriptContext;
	}

	uintptr_t GetGlobalState(uintptr_t scriptContextState) {
		int32_t state_identity = 0;
		uintptr_t state_actor = {};
		return RBX::GetGlobalState(scriptContextState, &state_identity, &state_actor);
	}
	
	uintptr_t DecryptLuaState(uintptr_t encryptedState) {
		return RBX::DecryptLuaState(encryptedState);
	}
}
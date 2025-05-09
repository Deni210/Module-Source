#pragma once

#include <vector>
#include <string>

#include <Luau/Compiler.h>
#include <Luau/BytecodeBuilder.h>
#include <Luau/BytecodeUtils.h>
#include <Luau/Bytecode.h>

#include <lapi.h>
#include <lstate.h>
#include <lualib.h>

#include <zstd/zstd.h>
#include <zstd/xxhash.h>

#include "Offsets/Offsets.hpp"

namespace Execution {
	void Execute(lua_State*, std::string);
	void SetProtoCapabilities(Proto*);
	std::string CompileScript(const std::string);
}
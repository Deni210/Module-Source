#pragma once

#include <lstate.h>
#include <lualib.h>
#include <lapi.h>

namespace Cache {
	int invalidate(lua_State* L) {
		SetFunction("cache::invalidate");
		luaL_checktype(L, 1, LUA_TUSERDATA);

		if (strcmp(luaL_typename(L, 1), "Instance") != LUA_OK)
		{
			luaL_typeerror(L, 1, "Instance");
			return 0;
		}

		const auto Instance = *reinterpret_cast<std::uintptr_t*>(lua_touserdata(L, 1));
		lua_pop(L, lua_gettop(L));

		lua_pushlightuserdata(L, (void*)Offsets::PushInstance);
		lua_gettable(L, LUA_REGISTRYINDEX);

		lua_pushlightuserdata(L, (void*)Instance);
		lua_pushnil(L);
		lua_settable(L, -3);

		return 0;
	}

	int iscached(lua_State* L) {
		SetFunction("cache::iscached");
		luaL_checktype(L, 1, LUA_TUSERDATA);

		if (strcmp(luaL_typename(L, 1), "Instance") != LUA_OK)
		{
			luaL_typeerror(L, 1, "Instance");
			return 0;
		}

		const auto Instance = *reinterpret_cast<std::uintptr_t*>(lua_touserdata(L, 1));

		lua_pushlightuserdata(L, (void*)Offsets::PushInstance);
		lua_gettable(L, LUA_REGISTRYINDEX);

		lua_pushlightuserdata(L, (void*)Instance);
		lua_gettable(L, -2);

		lua_pushboolean(L, !lua_isnil(L, -1));

		return 1;
	};

	int replace(lua_State* L) {
		SetFunction("cache::replace");
		luaL_checktype(L, 1, LUA_TUSERDATA);
		luaL_checktype(L, 2, LUA_TUSERDATA);

		if (strcmp(luaL_typename(L, 1), "Instance") != LUA_OK)
		{
			luaL_typeerror(L, 1, "Instance");
			return 0;
		}

		if (strcmp(luaL_typename(L, 2), "Instance") != LUA_OK)
		{
			luaL_typeerror(L, 2, "Instance");
			return 0;
		}

		const auto Instance = *reinterpret_cast<std::uintptr_t*>(lua_touserdata(L, 1));

		lua_pushlightuserdata(L, (void*)Offsets::PushInstance);
		lua_gettable(L, LUA_REGISTRYINDEX);

		lua_pushlightuserdata(L, (void*)Instance);
		lua_pushvalue(L, 2);
		lua_settable(L, -3);
		return 0;
	}

	int cloneref(lua_State* L) {
		SetFunction("cache::cloneref");
		luaL_checktype(L, 1, LUA_TUSERDATA);

		auto originalData = lua_touserdata(L, 1);
		uintptr_t userData = *(uintptr_t*)originalData;

		lua_pushlightuserdata(L, (void*)Offsets::PushInstance);
		lua_rawget(L, -10000);
		lua_pushlightuserdata(L, (void*)userData);
		lua_rawget(L, -2);

		lua_pushlightuserdata(L, (void*)userData);
		lua_pushnil(L);
		lua_rawset(L, -4);

		RBX::PushInstance(L, originalData);

		lua_pushlightuserdata(L, (void*)userData);
		lua_pushvalue(L, -3);
		lua_rawset(L, -5);

		return 1;
	}

	int compareinstances(lua_State* L) {
		SetFunction("cache::compareinstances");
		luaL_checktype(L, 1, LUA_TUSERDATA);
		luaL_checktype(L, 2, LUA_TUSERDATA);

		if (strcmp(luaL_typename(L, 1), "Instance") != LUA_OK)
		{
			luaL_typeerror(L, 1, "Instance");
			return 0;
		}

		if (strcmp(luaL_typename(L, 2), "Instance") != LUA_OK)
		{
			luaL_typeerror(L, 2, "Instance");
			return 0;
		}

		lua_pushboolean(L, (*(__int64*)lua_touserdata(L, 1) == *(__int64*)lua_touserdata(L, 2)));
		return 1;
	}
}
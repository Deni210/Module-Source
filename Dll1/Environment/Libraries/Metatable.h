#pragma once

#include <lstate.h>
#include <lualib.h>
#include <lapi.h>

namespace Metatable {
	int getrawmetatable(lua_State* L) {
		SetFunction("metatable::getrawmetatable");
		if (!lua_getmetatable(L, 1))
			lua_pushnil(L);

		return 1;
	}

	int getnamecallmethod(lua_State* L) {
		SetFunction("metatable::getnamecallmethod");
		if (L->namecall)
			lua_pushstring(L, L->namecall->data);
		else
			lua_pushnil(L);

		return 1;
	}

	int isreadonly(lua_State* L) {
		SetFunction("metatable::isreadonly");
		luaL_checktype(L, 1, LUA_TTABLE);

		LuaTable* table = hvalue(luaA_toobject(L, 1));

		lua_pushboolean(L, table->readonly);

		return 1;
	}

	int setrawmetatable(lua_State* L) {
		SetFunction("metatable::setrawmetatable");
		const auto ObjectType = lua_type(L, 1);
		if (ObjectType != LUA_TTABLE && ObjectType != LUA_TUSERDATA)
		{
			luaL_typeerror(L, 1, "table or userdata");
			return 0;
		}

		const auto MetatableType = lua_type(L, 2);
		if (MetatableType != LUA_TTABLE && MetatableType != LUA_TNIL)
		{
			luaL_typeerror(L, 2, "nil or table");
			return 0;
		}

		lua_settop(L, 2);
		lua_setmetatable(L, 1);

		lua_pushvalue(L, 1);

		return 1;
	}

	int setreadonly(lua_State* L) {
		SetFunction("metatable::setreadonly");
		luaL_checktype(L, 1, LUA_TTABLE);
		luaL_checktype(L, 2, LUA_TBOOLEAN);

		LuaTable* table = hvalue(luaA_toobject(L, 1));

		if (!CheckMemory((uintptr_t)table) || !CheckMemory((uintptr_t)table + offsetof(LuaTable, readonly)))
			return 0;

		table->readonly = lua_toboolean(L, 2);

		return 0;
	}

	int makereadonly(lua_State* L) {
		SetFunction("metatable::makereadonly");
		luaL_checktype(L, 1, LUA_TTABLE);

		LuaTable* table = hvalue(luaA_toobject(L, 1));

		table->readonly = true;

		return 0;
	}

	int makewriteable(lua_State* L) {
		SetFunction("metatable::makewritable");
		luaL_checktype(L, 1, LUA_TTABLE);

		LuaTable* table = hvalue(luaA_toobject(L, 1));

		table->readonly = false;

		return 0;
	}
}
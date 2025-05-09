#pragma once

#pragma comment(lib, "wininet.lib")

#include <lstate.h>
#include <lualib.h>
#include <lapi.h>
#include <lfunc.h>
#include <lgc.h>

#include <mutex>
#include "../../../Execution/Execution.hpp"
#include <Tools.hpp>
#include <format>
struct base_t;

template< class t >
class c_storage
{
	std::mutex m_l;

protected:
	t m_container;
public:

	auto safe_request(auto request, auto... args)
	{
		std::unique_lock l{ m_l };

		return request(args...);
	};

	void clear()
	{
		safe_request([&]()
			{ m_container.clear(); });
	}
};

using block_cache_t
= std::unordered_map< void*,
	bool >;

class c_blocked : public c_storage< block_cache_t >
{
public:
	void toggle(void* connection, bool enabled)
	{
		safe_request([&]()
			{
				m_container[connection] = enabled;
			});
	}

	bool should_block(void* connection)
	{
		return safe_request([&]()
			{
				return m_container.contains(connection)
					? !m_container[connection] : false;
			});
	}

} inline g_blocked;

using instance_cache_t
= std::unordered_map< void*,
	std::unordered_map< std::string, bool > >;

class c_instance_cache : public c_storage< instance_cache_t >
{
public:
	void toggle(void* instance, const std::string& prop, bool enabled)
	{
		safe_request([&]()
			{
				m_container[instance][prop] = enabled;
			});
	}

	std::optional< bool > is_scripLuaTable(void* instance, const std::string& prop)
	{
		return safe_request([&]() -> std::optional< bool >
			{
				if (m_container.contains(instance))
				{
					const auto properties = m_container[instance];

					if (properties.contains(prop))
						return properties.at(prop);
				}

				return std::nullopt;
			});
	}
} inline g_instance;

using closure_cache_t
= std::vector< void* >;

class c_closure_cache : public c_storage< closure_cache_t >
{
public:
	void add(void* closure)
	{
		safe_request([&]()
			{
				m_container.push_back(closure);
			});
	}

	bool contains(void* closure)
	{
		return safe_request([&]() -> bool
			{
				return std::find(m_container.begin(), m_container.end(), closure) != m_container.end();
			});
	}
} inline g_closure_cache;

using newcclosure_cache_t = std::unordered_map< Closure*, Closure* >;

class c_newcclosure_cache : public c_storage< newcclosure_cache_t >
{
public:
	void add(Closure* cclosure, Closure* lclosure)
	{
		safe_request([&]()
			{
				m_container[cclosure] = lclosure;
			});
	}

	void remove(Closure* obj)
	{
		safe_request([&]()
			{
				auto it = m_container.find(obj);
				if (it != m_container.end())
					m_container.erase(it);
			});
	}

	std::optional< Closure* > get(Closure* closure)
	{
		return safe_request([&]() -> std::optional< Closure* >
			{
				if (m_container.contains(closure))
					return m_container.at(closure);

				return std::nullopt;
			});
	}
} inline g_newcclosure_cache;

static void handler_run(lua_State* L, void* ud) {
	luaD_call(L, (StkId)(ud), LUA_MULTRET);
}

int handler_continuation(lua_State* L, std::int32_t status) {
	if (status != LUA_OK) {
		std::string regexed_error = lua_tostring(L, -1);
		lua_pop(L, 1);

		lua_pushlstring(L, regexed_error.c_str(), regexed_error.size());
		lua_error(L);
	}

	return lua_gettop(L);
};

int handler(lua_State* L) {
	const auto arg_count = lua_gettop(L);
	const auto closure = g_newcclosure_cache.get(clvalue(L->ci->func));

	if (!closure)
		luaL_error(L, "Failed to find closure");

	setclvalue(L, L->top, *closure);
	L->top++;

	lua_insert(L, 1);

	StkId func = L->base;
	L->ci->flags |= LUA_CALLINFO_HANDLE;

	L->baseCcalls++;
	int status = luaD_pcall(L, handler_run, func, savestack(L, func), 0);
	L->baseCcalls--;

	if (status == LUA_ERRRUN) {
		std::string regexed_error = lua_tostring(L, -1);
		lua_pop(L, 1);

		lua_pushlstring(L, regexed_error.c_str(), regexed_error.size());
		lua_error(L);
		return 0;
	}

	expandstacklimit(L, L->top);

	if (status == 0 && (L->status == LUA_YIELD || L->status == LUA_BREAK))
		return -1;

	return lua_gettop(L);
};

int wrapclosure(lua_State* L, int index) {
	luaL_checktype(L, index, LUA_TFUNCTION);

	lua_ref(L, index);
	lua_pushcclosurek(L, handler, nullptr, 0, handler_continuation);
	lua_ref(L, -1);

	g_newcclosure_cache.add(clvalue(luaA_toobject(L, -1)), clvalue(luaA_toobject(L, index)));

	return 1;
};

namespace ClosureLib {
	int checkcaller(lua_State* L) {
		SetFunction("closure::checkcaller");
		lua_pushboolean(L, L->userdata->Script.expired());

		return 1;
	}

	int clonefunction(lua_State* L) {
		SetFunction("closure::clonefunction");
		luaL_checktype(L, 1, LUA_TFUNCTION);

		Closure* toClone = clvalue(luaA_toobject(L, 1));

		if(toClone->isC)
			lua_clonecfunction(L, 1);
		else
			lua_clonefunction(L, 1);

		Closure* cloned = clvalue(luaA_toobject(L, -1));

		return 1;
	}

	int getcallingscript(lua_State* L) {
		SetFunction("closure::getcallingscript");
		uintptr_t userdata = (uintptr_t)L->userdata;
		__int64 scriptptr = *reinterpret_cast<uintptr_t*>(userdata + 0x50);

		if (scriptptr > 0)
			RBX::PushInstance(L, reinterpret_cast<__int64*>(userdata + 0x50));
		else
			lua_pushnil(L);

		return 1;
	}

	int iscclosure(lua_State* L) {
		SetFunction("closure::iscclosure");
		luaL_checktype(L, 1, LUA_TFUNCTION);

		Closure* closure = clvalue(luaA_toobject(L, 1));

		lua_pushboolean(L, closure->isC);

		return 1;
	}

	int islclosure(lua_State* L) {
		SetFunction("closure::islclosure");
		luaL_checktype(L, 1, LUA_TFUNCTION);

		Closure* closure = clvalue(luaA_toobject(L, 1));

		lua_pushboolean(L, !closure->isC);

		return 1;
	}

	int isexecutorclosure(lua_State* rl)
	{
		SetFunction("closure::isexecutorclosure");
		if (lua_type(rl, 1) != LUA_TFUNCTION) { lua_pushboolean(rl, false); return 1; }

		Closure* closure = clvalue(luaA_toobject(rl, 1));
		bool value = false;

		if (lua_isLfunction(rl, 1))
		{
			value = closure->l.p->linedefined;
		}
		else
		{
			const auto& ClosureList = GetClosureList();
			value = ClosureList.find(closure) != ClosureList.end();
		}

		lua_pushboolean(rl, value);
		return 1;
	}

	int isnewcclosure(lua_State* L) {
		SetFunction("closure::isexecutorclosure");
		if (lua_type(L, 1) != LUA_TFUNCTION) { lua_pushboolean(L, false); return 1; }

		Closure* cl = clvalue(luaA_toobject(L, 1));

		lua_pushboolean(L, g_newcclosure_cache.get(cl).has_value());

		return 1;
	}

	int loadstring(lua_State* L) {
		SetFunction("closure::loadstring");
		luaL_checktype(L, 1, LUA_TSTRING);

		const std::string source = lua_tostring(L, 1);
		const std::string chunkname = luaL_optstring(L, 2, "=");

		std::string script = Execution::CompileScript(source);

		if (script[0] == '\0' || script.empty()) {
			lua_pushnil(L);
			lua_pushstring(L, "Failed to compile script");
			return 2;
		}

		int result = RBX::LuaVM__Load(L, &script, chunkname.data(), 0);
		if (result != LUA_OK) {
			std::string Error = luaL_checklstring(L, -1, nullptr);
			lua_pop(L, 1);

			lua_pushnil(L);
			lua_pushstring(L, Error.data());

			return 2;
		}

		Closure* closure = clvalue(luaA_toobject(L, -1));

		Execution::SetProtoCapabilities(closure->l.p);

		return 1;
	}

	int newcclosure(lua_State* L) {
		SetFunction("closure::newcclosure");
		luaL_checktype(L, 1, LUA_TFUNCTION);

		if (!lua_iscfunction(L, 1))
			return wrapclosure(L, 1);

		lua_pushvalue(L, 1);
		return 1;
	};

	int hookmetamethod(lua_State* L)
	{
		SetFunction("closure::hookmetamethod");
		luaL_checkany(L, 1);

		const auto MetatableName = luaL_checkstring(L, 2);
		luaL_checktype(L, 3, LUA_TFUNCTION);

		lua_pushvalue(L, 1);
		lua_getmetatable(L, -1);
		if (lua_getfield(L, -1, MetatableName) == LUA_TNIL)
		{
			luaL_argerrorL(
				L, 2, std::format("'{}' is not a valid member of the given object's metatable.", MetatableName).c_str()
			);
		}
		lua_setreadonly(L, -2, false);

		lua_getglobal(L, "hookfunction");
		lua_pushvalue(L, -2);
		lua_pushvalue(L, 3);
		lua_call(L, 2, 1);
		lua_remove(L, -2);
		lua_setreadonly(L, -2, true);

		return 1;
	}
	

	int hookfunction(lua_State* L) {
		SetFunction("closure::hookfunction");

		if (!lua_isfunction(L, 1) || !lua_isfunction(L, 2)) {
			lua_pushstring(L, "Both arguments must be functions");
			lua_error(L);
			return 0;
		}

		Closure* Function = clvalue(luaA_toobject(L, 1));
		Closure* Hook = clvalue(luaA_toobject(L, 2));

		if (!Function || !Hook) {
			lua_pushstring(L, "Failed to find closure");
			lua_error(L);
			return 0;
		}

		lua_pushvalue(L, 1);

		if (Function->isC) {
			if (Hook->isC) {
				// C->C, C->NC, NC->C, NC->NC
				RBX::Print(1, "C->C, C->NC, NC->C, NC->NC");

				lua_CFunction Func1 = Hook->c.f;

				lua_clonecfunction(L, 1);
				lua_ref(L, -1);

				for (int i = 0; i < Hook->nupvalues; i++) {
					auto OldTValue = &Function->c.upvals[i];
					auto HookTValue = &Hook->c.upvals[i];
					OldTValue->value = HookTValue->value;
					OldTValue->tt = HookTValue->tt;
				}

				auto closureOpt = g_newcclosure_cache.get(Function);
				if (closureOpt.has_value()) {
					Closure* cachedClosure = closureOpt.value();
					g_newcclosure_cache.remove(Function);
					g_newcclosure_cache.add(Function, Hook);

					Closure* clonedClosure = clvalue(luaA_toobject(L, -1));
					if (clonedClosure) {
						g_newcclosure_cache.add(clonedClosure, cachedClosure);
					}
				}

				Function->nupvalues = Hook->nupvalues;
				Function->c.f = Func1;

				return 1;
			}
			else {
				// C->L, NC->L
				RBX::Print(1, "C->L, NC->L");

				wrapclosure(L, 2);
				lua_ref(L, -1);

				lua_clonecfunction(L, 1);
				lua_ref(L, -1);

				g_newcclosure_cache.add(Function, Hook);

				Function->c.f = reinterpret_cast<lua_CFunction>(handler);
				Function->c.cont = reinterpret_cast<lua_Continuation>(handler_continuation);

				return 1;
			}
		}
		else {
			if (Hook->isC) {
				// L->C
				lua_newtable(L);
				lua_newtable(L);
				lua_pushvalue(L, LUA_GLOBALSINDEX);
				lua_setfield(L, -2, "__index");
				lua_setreadonly(L, -1, true);
				lua_setmetatable(L, -2);

				lua_pushvalue(L, 2);
				lua_setfield(L, -2, "cFuncCall");

				std::string bytecode = Execution::CompileScript("return cFuncCall(...)");
				int loadResult = RBX::LuaVM__Load(L, &bytecode, "=", -1);

				if (loadResult != 0) {
					lua_pushstring(L, "Failed to compile wrapper for C function");
					lua_error(L);
					return 0;
				}

				Hook = clvalue(luaA_toobject(L, -1));

				if (!Hook) {
					lua_pushstring(L, "Failed to convert C function to Lua function");
					lua_error(L);
					return 0;
				}
			}

			// L->C, L->NC, L->L
			RBX::Print(1, "L->C, L->NC, L->L");

			lua_clonefunction(L, 1);

			Proto* newProto = Hook->l.p;

			Function->env = Hook->env;
			Function->stacksize = Hook->stacksize;
			Function->preload = Hook->preload;

			for (int i = 0; i < Hook->nupvalues; ++i) {
				setobj2n(L, &Function->l.uprefs[i], &Hook->l.uprefs[i]);
			}

			Function->nupvalues = Hook->nupvalues;
			Function->l.p = newProto;

			return 1;
		}
	}

}
#pragma once

#include <lstate.h>
#include <lz4.h>
#include <lualib.h>
#include <lapi.h>
#include <lmem.h>
#include <lfunc.h>
#include <lgc.h>
#include <algorithm>

namespace Debug {
    int getconstant(lua_State* L) {
        SetFunction("debug::getconstant");
        if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
            luaL_argerror(L, 1, "function or number expected");
            return 0;
        }

        const int index = luaL_checkinteger(L, 2);

        if (lua_isnumber(L, 1)) {
            lua_Debug ar;

            if (!lua_getinfo(L, (int)lua_tonumber(L, 1), "f", &ar)) {
                luaL_error(L, "level out of range");
                return 0;
            }

            if (lua_iscfunction(L, -1)) {
                luaL_argerror(L, 1, "stack points to a C closure, Lua function expected");
                return 0;
            }
        }
        else {
            lua_pushvalue(L, 1);

            if (lua_iscfunction(L, -1)) {
                luaL_argerror(L, 1, "Lua function expected");
                return 0;
            }
        }

        Closure* cl = (Closure*)lua_topointer(L, -1);
        Proto* p = cl->l.p;
        TValue* k = p->k;

        if (!index) {
            luaL_argerror(L, 1, "constant index starts at 1");
            return 0;
        }

        if (index > p->sizek) {
            lua_pushnil(L);
            return 1;
        }

        TValue* tval = &(k[index - 1]);

        if (tval->tt == LUA_TFUNCTION) {
            TValue* i_o = (L->top);
            setnilvalue(i_o);
            L->top++;
        }
        else {
            TValue* i_o = (L->top);
            i_o->value = tval->value;
            i_o->tt = tval->tt;
            L->top++;
        }

        return 1;
    }

	int getconstants(lua_State* L) {
        SetFunction("debug::getconstants");
        if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
            luaL_argerror(L, 1, "function or number expected");
            return 0;
        }

        if (lua_isnumber(L, 1)) {
            lua_Debug ar;

            if (!lua_getinfo(L, (int)lua_tonumber(L, 1), "f", &ar)) {
                luaL_error(L, "level out of range");
                return 0;
            }

            if (lua_iscfunction(L, -1)) {
                luaL_argerror(L, 1, "stack points to a C closure, Lua function expected");
                return 0;
            }
        }
        else {
            lua_pushvalue(L, 1);

            if (lua_iscfunction(L, -1)) {
                luaL_argerror(L, 1, "Lua function expected");
                return 0;
            }
        }

        Closure* cl = (Closure*)lua_topointer(L, -1);
        Proto* p = cl->l.p;
        TValue* k = p->k;

        lua_newtable(L);

        for (int i = 0; i < p->sizek; i++) {
            TValue* tval = &(k[i]);

            if (tval->tt == LUA_TFUNCTION) {
                TValue* i_o = (L->top);
                setnilvalue(i_o);
                L->top++;
            }
            else {
                TValue* i_o = (L->top);
                i_o->value = tval->value;
                i_o->tt = tval->tt;
                L->top++;
            }

            lua_rawseti(L, -2, (i + 1));
        }

        return 1;
	}

    int getinfo(lua_State* L) {
        SetFunction("debug::getinfo");
        if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
            luaL_argerror(L, 1, "function or number expected");
            return 0;
        }

        intptr_t level{};
        if (lua_isfunction(L, 1))
            level = -lua_gettop(L);

        lua_Debug ar;
        if (!lua_getinfo(L, level, "sluanf", &ar))
            luaL_argerror(L, 1, "invalid level");

        lua_newtable(L);

        lua_pushvalue(L, 1);
        lua_setfield(L, -2, "func");

        lua_pushinteger(L, ar.nupvals);
        lua_setfield(L, -2, "nups");

        lua_pushstring(L, ar.source);
        lua_setfield(L, -2, "source");

        lua_pushstring(L, ar.short_src);
        lua_setfield(L, -2, "short_src");

        lua_pushinteger(L, ar.currentline);
        lua_setfield(L, -2, "currentline");

        lua_pushstring(L, ar.what);
        lua_setfield(L, -2, "what");

        lua_pushinteger(L, ar.linedefined);
        lua_setfield(L, -2, "linedefined");

        lua_pushinteger(L, ar.isvararg);
        lua_setfield(L, -2, "is_vararg");

        lua_pushinteger(L, ar.nparams);
        lua_setfield(L, -2, "numparams");

        lua_pushstring(L, ar.name);
        lua_setfield(L, -2, "name");

        if (lua_isfunction(L, 1) && lua_isLfunction(L, 1)) {
            Closure* cl = clvalue(luaA_toobject(L, 1));

            lua_pushinteger(L, cl->l.p->sizep);
            lua_setfield(L, -2, "sizep");
        }

        return 1;
    }

    int getproto(lua_State* L) {
        SetFunction("debug::getproto");
        if (lua_iscfunction(L, -1)) {
            luaL_argerror(L, 1, "stack points to a C closure, Lua function expected");
            return 0;
        }

        Closure* cl = nullptr;
        if (lua_isnumber(L, 1)) {
            lua_Debug ar;
            if (!lua_getinfo(L, luaL_checkinteger(L, 1), "f", &ar))
                luaL_argerror(L, 1, "level out of range");
            if (lua_iscfunction(L, -1))
                luaL_argerror(L, 1, "level points to cclosure");
            cl = (Closure*)lua_topointer(L, -1);
        }
        else if (lua_isfunction(L, 1)) {
            luaL_checktype(L, 1, LUA_TFUNCTION);
            cl = (Closure*)lua_topointer(L, 1);
            if (cl->isC)
                luaL_argerror(L, 1, "lclosure expected");
        }
        else {
            luaL_argerror(L, 1, "function or number expected");
        }

        int index = std::clamp(luaL_checkinteger(L, 2), 0, cl->l.p->sizep);
        bool active = false;
        if (!lua_isnoneornil(L, 3))
            active = luaL_checkboolean(L, 3);
        if (!active) {
            if (index == 0 && cl->l.p->sizep == 0) {
                lua_pop(L, lua_gettop(L));
                lua_pushcclosure(L, [](lua_State* L) -> int {
                    return 0;
                }, 0, 0);

                return 1;
            }

            if (index < 1 || index > cl->l.p->sizep)
                luaL_error(L, "index out of range %d %d", index, cl->l.p->sizep);
            Proto* p = cl->l.p->p[index - 1];
            std::unique_ptr<TValue> function(new TValue{});
            Closure* clos = luaF_newLclosure(L, 0, cl->env, p);
            setclvalue(L, function.get(), clos);
            luaA_pushobject(L, function.get());
        }
        else {
            lua_newtable(L);

            struct Ctx {
                lua_State* L;
                int count;
                Closure* cl;
            } ctx{ L, 0, cl };

            luaM_visitgco(L, &ctx, [](void* pctx, lua_Page* page, GCObject* gco) -> bool {
                Ctx* ctx = static_cast<Ctx*>(pctx);
                if (!((gco->gch.marked ^ WHITEBITS) & otherwhite(ctx->L->global)))
                    return false;

                uint8_t tt = gco->gch.tt;
                if (tt == LUA_TFUNCTION) {
                    Closure* cl = (Closure*)gco;
                    if (!cl->isC && cl->l.p == ctx->cl->l.p->p[ctx->count]) {
                        setclvalue(ctx->L, ctx->L->top, cl);
                        ctx->L->top++;
                        lua_rawseti(ctx->L, -2, ++ctx->count);
                    }
                }
                return false;
            });
        }
        return 1;
    }

    int getprotos(lua_State* L) {
        SetFunction("debug::getprotos");
        luaL_checktype(L, 1, LUA_TFUNCTION);

        if (lua_iscfunction(L, 1))
            luaL_argerror(L, 1, "Lua function expected");

        lua_Debug ar;
        if (!lua_getinfo(L, -1, "f", &ar))
            luaL_error(L, "invalid level passed to getprotos");

        Closure* cl = clvalue(luaA_toobject(L, -1));
        if (cl->isC) {
            lua_pop(L, 1);
            lua_newtable(L);
            return 1;
        }
        lua_pop(L, 1);

        lua_newtable(L);
        int idx = 1;
        for (auto i = 0; i < cl->l.p->sizep; ++i) {
            const auto proto = cl->l.p->p[i];
            lua_checkstack(L, 1);

            if (proto->nups <= 0) {
                Closure* clos = luaF_newLclosure(L, NULL, cl->env, proto);
                setclvalue(L, L->top, clos);
                L->top++;
            }
            else {
                lua_pushcclosure(L, [](lua_State* L) {
                    return 0;
                }, 0, 0);

            }
            lua_rawseti(L, -2, idx++);
        }

        return 1;
    }

    int getstack(lua_State* L) {
        SetFunction("debug::getstack");
        luaL_checktype(L, 1, LUA_TNUMBER);

        const auto level = lua_tointeger(L, 1);
        const auto index = luaL_optinteger(L, 2, -1);

        if (level >= L->ci - L->base_ci || level < 0) {
            luaL_argerror(L, 1, "level out of range");
        }

        const auto frame = reinterpret_cast<CallInfo*>(L->ci - level);
        const auto top = (frame->top - frame->base);

        if (clvalue(frame->func)->isC) {
            luaL_argerror(L, 1, "level points to a cclosure, lclosure expected");
        }

        if (index == -1) {
            lua_newtable(L);

            for (int i = 0; i < top; i++) {
                setobj2s(L, L->top, &frame->base[i]);
                L->top++;

                lua_rawseti(L, -2, i + 1);
            }
        }
        else {
            if (index < 1 || index > top) {
                luaL_argerror(L, 2, "stack index out of range");
            }

            setobj2s(L, L->top, &frame->base[index - 1]);
            L->top++;
        }
        return 1;
    }

    int getupvalue(lua_State* L) {
        SetFunction("debug::getupvalue");
        if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
            luaL_argerror(L, 1, "function or number expected");
            return 0;
        }

        if (lua_isnumber(L, 1)) {
            lua_Debug ar;

            if (!lua_getinfo(L, (int)lua_tonumber(L, 1), "f", &ar)) {
                luaL_error(L, "level out of range");
                return 0;
            }
        }
        else
            lua_pushvalue(L, 1);

        const int index = luaL_checkinteger(L, 2);

        Closure* closure = (Closure*)lua_topointer(L, -1);
        TValue* upvalue_table = (TValue*)nullptr;

        if (!closure->isC)
            upvalue_table = closure->l.uprefs;
        else if (closure->isC)
            upvalue_table = closure->c.upvals;

        if (!index) {
            luaL_argerror(L, 1, "upvalue index starts at 1");
            return 0;
        }

        if (index > closure->nupvalues) {
            luaL_argerror(L, 1, "upvalue index is out of range");
            return 0;
        }

        TValue* upval = (&upvalue_table[index - 1]);
        TValue* top = L->top;

        top->value = upval->value;
        top->tt = upval->tt;
        L->top++;

        return 1;
    }

    int getupvalues(lua_State* L) {
        SetFunction("debug::getupvalues");
        if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
            luaL_argerror(L, 1, "function or number expected");
            return 0;
        }

        if (lua_isnumber(L, 1)) {
            lua_Debug ar;

            if (!lua_getinfo(L, (int)lua_tonumber(L, 1), "f", &ar)) {
                luaL_error(L, "level out of range");
                return 0;
            }
        }
        else {
            lua_pushvalue(L, 1);
        }

        Closure* closure = (Closure*)lua_topointer(L, -1);
        TValue* upvalue_table = (TValue*)nullptr;

        lua_newtable(L);

        if (!closure->isC)
            upvalue_table = closure->l.uprefs;
        else if (closure->isC)
            upvalue_table = closure->c.upvals;

        for (int i = 0; i < closure->nupvalues; i++) {
            TValue* upval = (&upvalue_table[i]);
            TValue* top = L->top;

            top->value = upval->value;
            top->tt = upval->tt;
            L->top++;

            lua_rawseti(L, -2, (i + 1));
        }

        return 1;
    }

    int setconstant(lua_State* L) {
        SetFunction("debug::setconstant");
        if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
            luaL_argerror(L, 1, "function or number expected");
            return 0;
        }

        const int index = luaL_checkinteger(L, 2);

        luaL_checkany(L, 3);

        if (lua_isnumber(L, 1)) {
            lua_Debug ar;

            if (!lua_getinfo(L, (int)lua_tonumber(L, 1), "f", &ar)) {
                luaL_error(L, "level out of range");
                return 0;
            }

            if (lua_iscfunction(L, -1)) {
                luaL_argerror(L, 1, "stack points to a C closure, Lua function expected");
                return 0;
            }
        }
        else {
            lua_pushvalue(L, 1);

            if (lua_iscfunction(L, -1)) {
                luaL_argerror(L, 1, "Lua function expected");
                return 0;
            }
        }

        Closure* cl = (Closure*)lua_topointer(L, -1);
        Proto* p = cl->l.p;
        TValue* k = p->k;

        if (!index) {
            luaL_argerror(L, 1, "constant index starts at 1");
            return 0;
        }

        if (index > p->sizek) {
            luaL_argerror(L, 1, "constant index is out of range");
            return 0;
        }

        auto constant = &k[index - 1]; /* Lua-based indexing */

        if (constant->tt == LUA_TFUNCTION) {
            return 0;
        }

        const TValue* new_t = luaA_toobject(L, 3);
        constant->tt = new_t->tt;
        constant->value = new_t->value;

        return 0;
    }

    int setstack(lua_State* L) {
        SetFunction("debug::setstack");
        luaL_checktype(L, 1, LUA_TNUMBER);
        luaL_checktype(L, 2, LUA_TNUMBER);
        luaL_checkany(L, 3);

        const auto level = lua_tointeger(L, 1);
        const auto index = lua_tointeger(L, 2);

        if (level >= L->ci - L->base_ci || level < 0) {
            luaL_argerror(L, 1, "level out of range");
        }

        const auto frame = reinterpret_cast<CallInfo*>(L->ci - level);
        const auto top = (frame->top - frame->base);

        if (clvalue(frame->func)->isC) {
            luaL_argerror(L, 1, "level points to a cclosure, lclosure expected");
        }

        if (index < 1 || index > top) {
            luaL_argerror(L, 2, "stack index out of range");
        }

        setobj2s(L, &frame->base[index - 1], luaA_toobject(L, 3));
        return 0;
    }
    
    int _setupvalue(lua_State* L) {
        if (lua_isfunction(L, 1) == false && lua_isnumber(L, 1) == false) {
            luaL_argerror(L, 1, "function or number expected");
            return 0;
        }

        const int index = luaL_checkinteger(L, 2);
        luaL_checkany(L, 3);

        if (lua_isnumber(L, 1)) {
            lua_Debug ar;

            if (!lua_getinfo(L, (int)lua_tonumber(L, 1), "f", &ar)) {
                luaL_error(L, "level out of range");
                return 0;
            }
        }
        else
            lua_pushvalue(L, 1);

        Closure* closure = (Closure*)lua_topointer(L, -1);
        const TValue* value = luaA_toobject(L, 3);
        TValue* upvalue_table = (TValue*)nullptr;

        if (!closure->isC)
            upvalue_table = closure->l.uprefs;
        else if (closure->isC)
            upvalue_table = closure->c.upvals;

        if (!index) {
            luaL_argerror(L, 1, "upvalue index starts at 1");
            return 0;
        }

        if (index > closure->nupvalues) {
            luaL_argerror(L, 1, "upvalue index is out of range");
            return 0;
        }

        TValue* upvalue = (&upvalue_table[index - 1]);

        upvalue->value = value->value;
        upvalue->tt = value->tt;

        luaC_barrier(L, closure, value);

        return 0;
    }
}
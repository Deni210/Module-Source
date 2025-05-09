// This file is part of the Luau programming language and is licensed under MIT License; see LICENSE.txt for details
// This code is based on Lua 5.x implementation licensed under MIT License; see lua_LICENSE.txt for details
#include "lvm.h"

#include "lstate.h"
#include "ltable.h"
#include "lfunc.h"
#include "lstring.h"
#include "lgc.h"
#include "lmem.h"
#include "ldebug.h"
#include "ldo.h"
#include "lbuiltins.h"
#include "lnumutils.h"
#include "lbytecode.h"

#include <string.h>

// Disable c99-designator to avoid the warning in CGOTO dispatch table
#ifdef __clang__
#if __has_warning("-Wc99-designator")
#pragma clang diagnostic ignored "-Wc99-designator"
#endif
#endif

// When working with VM code, pay attention to these rules for correctness:
// 1. Many external Lua functions can fail; for them to fail and be able to generate a proper stack, we need to copy pc to L->ci->savedpc before the
// call
// 2. Many external Lua functions can reallocate the stack. This invalidates stack pointers in VM C stack frame, most importantly base, but also
// ra/rb/rc!
// 3. VM_PROTECT macro saves savedpc and restores base for you; most external calls need to be wrapped into that. However, it does NOT restore
// ra/rb/rc!
// 4. When copying an object to any existing object as a field, generally speaking you need to call luaC_barrier! Be careful with all setobj calls
// 5. To make 4 easier to follow, please use setobj2s for copies to stack, setobj2t for writes to tables, and setobj for other copies.
// 6. You can define HARDSTACKTESTS in llimits.h which will aggressively realloc stack; with address sanitizer this should be effective at finding
// stack corruption bugs
// 7. Many external Lua functions can call GC! GC will *not* traverse pointers to new objects that aren't reachable from Lua root. Be careful when
// creating new Lua objects, store them to stack soon.

// When calling luau_callTM, we usually push the arguments to the top of the stack.
// This is safe to do for complicated reasons:
// - stack guarantees EXTRA_STACK room beyond stack_last (see luaD_reallocstack)
// - stack reallocation copies values past stack_last

// All external function calls that can cause stack realloc or Lua calls have to be wrapped in VM_PROTECT
// This makes sure that we save the pc (in case the Lua call needs to generate a backtrace) before the call,
// and restores the stack pointer after in case stack gets reallocated
// Should only be used on the slow paths.
#define VM_PROTECT(x) \
    { \
        L->ci->savedpc = pc; \
        { \
            x; \
        }; \
        base = L->base; \
    }

// Some external functions can cause an error, but never reallocate the stack; for these, VM_PROTECT_PC() is
// a cheaper version of VM_PROTECT that can be called before the external call.
#define VM_PROTECT_PC() L->ci->savedpc = pc

#define VM_REG(i) (LUAU_ASSERT(unsigned(i) < unsigned(L->top - base)), &base[i])
#define VM_KV(i) (LUAU_ASSERT(unsigned(i) < unsigned(cl->l.p->sizek)), &k[i])
#define VM_UV(i) (LUAU_ASSERT(unsigned(i) < unsigned(cl->nupvalues)), &cl->l.uprefs[i])

#define VM_PATCH_C(pc, slot) *const_cast<Instruction*>(pc) = ((uint8_t(slot) << 24) | (0x00ffffffu & *(pc)))
#define VM_PATCH_E(pc, slot) *const_cast<Instruction*>(pc) = ((uint32_t(slot) << 8) | (0x000000ffu & *(pc)))

#define VM_INTERRUPT() \
    { \
        void (*interrupt)(lua_State*, int) = L->global->cb.interrupt; \
        if (LUAU_UNLIKELY(!!interrupt)) \
        { /* the interrupt hook is called right before we advance pc */ \
            VM_PROTECT(L->ci->savedpc++; interrupt(L, -1)); \
            if (L->status != 0) \
            { \
                L->ci->savedpc--; \
                goto exit; \
            } \
        } \
    }

#define VM_DISPATCH_OP(op) &&CASE_##op

#define VM_DISPATCH_TABLE() \
    VM_DISPATCH_OP(LOP_NOP), VM_DISPATCH_OP(LOP_BREAK), VM_DISPATCH_OP(LOP_LOADNIL), VM_DISPATCH_OP(LOP_LOADB), VM_DISPATCH_OP(LOP_LOADN), \
        VM_DISPATCH_OP(LOP_LOADK), VM_DISPATCH_OP(LOP_MOVE), VM_DISPATCH_OP(LOP_GETGLOBAL), VM_DISPATCH_OP(LOP_SETGLOBAL), \
        VM_DISPATCH_OP(LOP_GETUPVAL), VM_DISPATCH_OP(LOP_SETUPVAL), VM_DISPATCH_OP(LOP_CLOSEUPVALS), VM_DISPATCH_OP(LOP_GETIMPORT), \
        VM_DISPATCH_OP(LOP_GETTABLE), VM_DISPATCH_OP(LOP_SETTABLE), VM_DISPATCH_OP(LOP_GETTABLEKS), VM_DISPATCH_OP(LOP_SETTABLEKS), \
        VM_DISPATCH_OP(LOP_GETTABLEN), VM_DISPATCH_OP(LOP_SETTABLEN), VM_DISPATCH_OP(LOP_NEWCLOSURE), VM_DISPATCH_OP(LOP_NAMECALL), \
        VM_DISPATCH_OP(LOP_CALL), VM_DISPATCH_OP(LOP_RETURN), VM_DISPATCH_OP(LOP_JUMP), VM_DISPATCH_OP(LOP_JUMPBACK), VM_DISPATCH_OP(LOP_JUMPIF), \
        VM_DISPATCH_OP(LOP_JUMPIFNOT), VM_DISPATCH_OP(LOP_JUMPIFEQ), VM_DISPATCH_OP(LOP_JUMPIFLE), VM_DISPATCH_OP(LOP_JUMPIFLT), \
        VM_DISPATCH_OP(LOP_JUMPIFNOTEQ), VM_DISPATCH_OP(LOP_JUMPIFNOTLE), VM_DISPATCH_OP(LOP_JUMPIFNOTLT), VM_DISPATCH_OP(LOP_ADD), \
        VM_DISPATCH_OP(LOP_SUB), VM_DISPATCH_OP(LOP_MUL), VM_DISPATCH_OP(LOP_DIV), VM_DISPATCH_OP(LOP_MOD), VM_DISPATCH_OP(LOP_POW), \
        VM_DISPATCH_OP(LOP_ADDK), VM_DISPATCH_OP(LOP_SUBK), VM_DISPATCH_OP(LOP_MULK), VM_DISPATCH_OP(LOP_DIVK), VM_DISPATCH_OP(LOP_MODK), \
        VM_DISPATCH_OP(LOP_POWK), VM_DISPATCH_OP(LOP_AND), VM_DISPATCH_OP(LOP_OR), VM_DISPATCH_OP(LOP_ANDK), VM_DISPATCH_OP(LOP_ORK), \
        VM_DISPATCH_OP(LOP_CONCAT), VM_DISPATCH_OP(LOP_NOT), VM_DISPATCH_OP(LOP_MINUS), VM_DISPATCH_OP(LOP_LENGTH), VM_DISPATCH_OP(LOP_NEWTABLE), \
        VM_DISPATCH_OP(LOP_DUPTABLE), VM_DISPATCH_OP(LOP_SETLIST), VM_DISPATCH_OP(LOP_FORNPREP), VM_DISPATCH_OP(LOP_FORNLOOP), \
        VM_DISPATCH_OP(LOP_FORGLOOP), VM_DISPATCH_OP(LOP_FORGPREP_INEXT), VM_DISPATCH_OP(LOP_FASTCALL3), VM_DISPATCH_OP(LOP_FORGPREP_NEXT), \
        VM_DISPATCH_OP(LOP_NATIVECALL), VM_DISPATCH_OP(LOP_GETVARARGS), VM_DISPATCH_OP(LOP_DUPCLOSURE), VM_DISPATCH_OP(LOP_PREPVARARGS), \
        VM_DISPATCH_OP(LOP_LOADKX), VM_DISPATCH_OP(LOP_JUMPX), VM_DISPATCH_OP(LOP_FASTCALL), VM_DISPATCH_OP(LOP_COVERAGE), \
        VM_DISPATCH_OP(LOP_CAPTURE), VM_DISPATCH_OP(LOP_SUBRK), VM_DISPATCH_OP(LOP_DIVRK), VM_DISPATCH_OP(LOP_FASTCALL1), \
        VM_DISPATCH_OP(LOP_FASTCALL2), VM_DISPATCH_OP(LOP_FASTCALL2K), VM_DISPATCH_OP(LOP_FORGPREP), VM_DISPATCH_OP(LOP_JUMPXEQKNIL), \
        VM_DISPATCH_OP(LOP_JUMPXEQKB), VM_DISPATCH_OP(LOP_JUMPXEQKN), VM_DISPATCH_OP(LOP_JUMPXEQKS), VM_DISPATCH_OP(LOP_IDIV), \
        VM_DISPATCH_OP(LOP_IDIVK),

#if defined(__GNUC__) || defined(__clang__)
#define VM_USE_CGOTO 1
#else
#define VM_USE_CGOTO 0
#endif

/**
 * These macros help dispatching Luau opcodes using either case
 * statements or computed goto.
 * VM_CASE(op) Generates either a case statement or a label
 * VM_NEXT() fetch a byte and dispatch or jump to the beginning of the switch statement
 * VM_CONTINUE() Use an opcode override to dispatch with computed goto or
 * switch statement to skip a LOP_BREAK instruction.
 */
#if VM_USE_CGOTO
#define VM_CASE(op) CASE_##op:
#define VM_NEXT() goto*(SingleStep ? &&dispatch : kDispatchTable[LUAU_INSN_OP(*pc)])
#define VM_CONTINUE(op) goto* kDispatchTable[uint8_t(op)]
#else
#define VM_CASE(op) case op:
#define VM_NEXT() goto dispatch
#define VM_CONTINUE(op) \
    dispatchOp = uint8_t(op); \
    goto dispatchContinue
#endif

 // Does VM support native execution via ExecutionCallbacks? We mostly assume it does but keep the define to make it easy to quantify the cost.
#define VM_HAS_NATIVE 1

LUAU_NOINLINE void luau_callhook(lua_State* L, lua_Hook hook, void* userdata)
{
    ptrdiff_t base = savestack(L, L->base);
    ptrdiff_t top = savestack(L, L->top);
    ptrdiff_t ci_top = savestack(L, L->ci->top);
    int status = L->status;

    // if the hook is called externally on a paused thread, we need to make sure the paused thread can emit Lua calls
    if (status == LUA_YIELD || status == LUA_BREAK)
    {
        L->status = 0;
        L->base = L->ci->base;
    }

    // note: the pc expectations of the hook are matching the general "pc points to next instruction"
    // however, for the hook to be able to continue execution from the same point, this is called with savedpc at the *current* instruction
    // this needs to be called before luaD_checkstack in case it fails to reallocate stack
    if (L->ci->savedpc)
        L->ci->savedpc++;

    luaD_checkstack(L, LUA_MINSTACK); // ensure minimum stack size
    L->ci->top = L->top + LUA_MINSTACK;
    LUAU_ASSERT(L->ci->top <= L->stack_last);

    Closure* cl = clvalue(L->ci->func);

    lua_Debug ar;
    ar.currentline = cl->isC ? -1 : luaG_getline(cl->l.p, pcRel(L->ci->savedpc, cl->l.p));
    ar.userdata = userdata;

    hook(L, &ar);

    if (L->ci->savedpc)
        L->ci->savedpc--;

    L->ci->top = restorestack(L, ci_top);
    L->top = restorestack(L, top);

    // note that we only restore the paused state if the hook hasn't yielded by itself
    if (status == LUA_YIELD && L->status != LUA_YIELD)
    {
        L->status = LUA_YIELD;
        L->base = restorestack(L, base);
    }
    else if (status == LUA_BREAK)
    {
        LUAU_ASSERT(L->status != LUA_BREAK); // hook shouldn't break again

        L->status = LUA_BREAK;
        L->base = restorestack(L, base);
    }
}

inline bool luau_skipstep(uint8_t op)
{
    return op == LOP_PREPVARARGS || op == LOP_BREAK;
}

void luau_execute(lua_State* L)
{
    using _Luau_Execute = void(__fastcall*)(lua_State*);
    auto Luau_Execute = (_Luau_Execute)Offsets::Luau_Execute;

    Luau_Execute(L); /* Handles both singlestep and non-singlestep conditions */
}

int luau_precall(lua_State* L, StkId func, int nresults)
{
    if (!ttisfunction(func))
    {
        luaV_tryfuncTM(L, func);
        // L->top is incremented by tryfuncTM
    }

    Closure* ccl = clvalue(func);

    CallInfo* ci = incr_ci(L);
    ci->func = func;
    ci->base = func + 1;
    ci->top = L->top + ccl->stacksize;
    ci->savedpc = NULL;
    ci->flags = 0;
    ci->nresults = nresults;

    L->base = ci->base;
    // Note: L->top is assigned externally

    luaD_checkstack(L, ccl->stacksize);
    LUAU_ASSERT(ci->top <= L->stack_last);

    if (!ccl->isC)
    {
        Proto* p = ccl->l.p;

        // fill unused parameters with nil
        StkId argi = L->top;
        StkId argend = L->base + p->numparams;
        while (argi < argend)
            setnilvalue(argi++); // complete missing arguments
        L->top = p->is_vararg ? argi : ci->top;

        ci->savedpc = p->code;

#if VM_HAS_NATIVE
        if (p->exectarget != 0 && p->execdata)
            ci->flags = LUA_CALLINFO_NATIVE;
#endif

        return PCRLUA;
    }
    else
    {
        lua_CFunction func = ccl->c.f;
        int n = func(L);

        // yield
        if (n < 0)
            return PCRYIELD;

        // ci is our callinfo, cip is our parent
        CallInfo* ci = L->ci;
        CallInfo* cip = ci - 1;

        // copy return values into parent stack (but only up to nresults!), fill the rest with nil
        // TODO: it might be worthwhile to handle the case when nresults==b explicitly?
        StkId res = ci->func;
        StkId vali = L->top - n;
        StkId valend = L->top;

        int i;
        for (i = nresults; i != 0 && vali < valend; i--)
            setobj2s(L, res++, vali++);
        while (i-- > 0)
            setnilvalue(res++);

        // pop the stack frame
        L->ci = cip;
        L->base = cip->base;
        L->top = res;

        return PCRC;
    }
}

void luau_poscall(lua_State* L, StkId first)
{
    // finish interrupted execution of `OP_CALL'
    // ci is our callinfo, cip is our parent
    CallInfo* ci = L->ci;
    CallInfo* cip = ci - 1;

    // copy return values into parent stack (but only up to nresults!), fill the rest with nil
    // TODO: it might be worthwhile to handle the case when nresults==b explicitly?
    StkId res = ci->func;
    StkId vali = first;
    StkId valend = L->top;

    int i;
    for (i = ci->nresults; i != 0 && vali < valend; i--)
        setobj2s(L, res++, vali++);
    while (i-- > 0)
        setnilvalue(res++);

    // pop the stack frame
    L->ci = cip;
    L->base = cip->base;
    L->top = (ci->nresults == LUA_MULTRET) ? res : cip->top;
}

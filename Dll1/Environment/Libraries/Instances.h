#pragma once

#include <lstate.h>
#include <lualib.h>
#include <lapi.h>
#include <lmem.h>

#include <bitset>

std::vector<std::tuple<uintptr_t, std::string, bool>> script_able_cache;
std::vector<std::pair<std::string, bool>> default_property_states;

int getCachedScriptableProperty(uintptr_t instance, std::string property) {
    for (auto& cacheData : script_able_cache) {
        uintptr_t instanceAddress = std::get<0>(cacheData);
        std::string instanceProperty = std::get<1>(cacheData);

        if (instanceAddress == instance && instanceProperty == property) {
            return std::get<2>(cacheData);
        }
    }

    return -1;
};

int getCachedDefultScriptableProperty(std::string property) {
    for (auto& cacheData : default_property_states) {
        if (cacheData.first == property) {
            return cacheData.second;
        }
    }

    return -1;
};

bool findAndUpdateScriptAbleCache(uintptr_t instance, std::string property, bool state) {
    for (auto& cacheData : script_able_cache) {
        uintptr_t instanceAddress = std::get<0>(cacheData);
        std::string instanceProperty = std::get<1>(cacheData);

        if (instanceAddress == instance && instanceProperty == property) {
            std::get<2>(cacheData) = state;
            return true;
        }
    }

    return false;
}

void addDefaultPropertyState(std::string property, bool state) {
    bool hasDefault = false;

    for (auto& cacheData : default_property_states) {
        if (cacheData.first == property) {
            hasDefault = true;
            break;
        }
    }

    if (!hasDefault) {
        default_property_states.push_back({ property, state });
    }
};

namespace RBX
{

    enum FastVarType : __int32
    {
        FASTVARTYPE_INVALID = 0x0,
        FASTVARTYPE_STATIC = 0x1,
        FASTVARTYPE_DYNAMIC = 0x2,
        FASTVARTYPE_SYNC = 0x4,
        FASTVARTYPE_AB_NEWUSERS = 0x8,
        FASTVARTYPE_AB_NEWSTUDIOUSERS = 0x10,
        FASTVARTYPE_AB_ALLUSERS = 0x20,
        FASTVARTYPE_LOCAL_LOCKED = 0x40,
        FASTVARTYPE_ANY = 0x7F,
    };

    struct LiveThreadRef {
        int __atomic_refs;
        lua_State* Thread;
        int ThreadId;
        int RefId;
    };

    struct WeakObjectRef {
    };

    struct WeakThreadRef {
        //lua_State* thread;
        std::uint8_t pad_0[16];

        WeakThreadRef* Previous;
        WeakThreadRef* Next;
        LiveThreadRef* LiveThreadRef;
        struct Node_t* Node;

        std::uint8_t pad_1[8];
    };

    struct Node {
        std::uint8_t pad_0[8];

        WeakThreadRef* WeakThreadRef;
    };

    struct ScriptVmState {
        void* Fill;
    };

    struct ExtraSpace {
        struct Shared {
            int32_t ThreadCount;
            void* ScriptContext;
            void* ScriptVmState;
            char field_18[0x8];
            void* __intrusive_set_AllThreads;
        };

        char _0[8];
        char _8[8];
        char _10[8];
        struct Shared* SharedExtraSpace;
    };

    struct WeakThreadRef2 {
        std::atomic< std::int32_t > _refs;
        lua_State* Thread;
        std::int32_t ThreadRef;
        std::int32_t ObjectId;
        std::int32_t unk1;
        std::int32_t unk2;

        WeakThreadRef2(lua_State* L)
            : Thread(L), ThreadRef(NULL), ObjectId(NULL), unk1(NULL), unk2(NULL) {
        };
    };

    struct DebuggerResult {
        std::int32_t Result;
        std::int32_t unk[0x4];
    };

    namespace Concepts
    {
        template<typename Derived, typename Base>
        concept _TypeConstraint = std::is_base_of_v<Base, Derived>;
    }

    namespace Security
    {
        enum Permissions : uint32_t
        {
            None = 0x0,
            Plugin = 0x1,
            LocalUser = 0x3,
            WritePlayer = 0x4,
            RobloxScript = 0x5,
            RobloxEngine = 0x6,
            NotAccessible = 0x7,
            TestLocalUser = 0x3
        };
    }

    struct PeerId
    {
        unsigned int peerId;
    };

    const struct SystemAddress
    {
        RBX::PeerId remoteId;
    };

    namespace Reflection
    {
        namespace EventSource {

            enum EventTargetInclusion : __int32
            {
                OnlyTarget = 0x0,
                ExcludeTarget = 0x1,
            };

            const struct __declspec(align(8)) RemoteEventInvocationTargetOptions
            {
                const RBX::SystemAddress* target;
                RBX::Reflection::EventSource::EventTargetInclusion isExcludeTarget;
            };
        }
        const struct type_holder
        {
            void(__fastcall* construct)(const char*, char*);
            void(__fastcall* moveConstruct)(char*, char*);
            void(__fastcall* destruct)(char*);
        };

        enum ReflectionType : uint32_t
        {
            ReflectionType_Void = 0x0,
            ReflectionType_Bool = 0x1,
            ReflectionType_Int = 0x2,
            ReflectionType_Int64 = 0x3,
            ReflectionType_Float = 0x4,
            ReflectionType_Double = 0x5,
            ReflectionType_String = 0x6,
            ReflectionType_ProtectedString = 0x7,
            ReflectionType_Instance = 0x8,
            ReflectionType_Instances = 0x9,
            ReflectionType_Ray = 0xa,
            ReflectionType_Vector2 = 0xb,
            ReflectionType_Vector3 = 0xc,
            ReflectionType_Vector2Int16 = 0xd,
            ReflectionType_Vector3Int16 = 0xe,
            ReflectionType_Rect2d = 0xf,
            ReflectionType_CoordinateFrame = 0x10,
            ReflectionType_Color3 = 0x11,
            ReflectionType_Color3uint8 = 0x12,
            ReflectionType_UDim = 0x13,
            ReflectionType_UDim2 = 0x14,
            ReflectionType_Faces = 0x15,
            ReflectionType_Axes = 0x16,
            ReflectionType_Region3 = 0x17,
            ReflectionType_Region3Int16 = 0x18,
            ReflectionType_CellId = 0x19,
            ReflectionType_GuidData = 0x1a,
            ReflectionType_PhysicalProperties = 0x1b,
            ReflectionType_BrickColor = 0x1c,
            ReflectionType_SystemAddress = 0x1d,
            ReflectionType_BinaryString = 0x1e,
            ReflectionType_Surface = 0x1f,
            ReflectionType_Enum = 0x20,
            ReflectionType_Property = 0x21,
            ReflectionType_Tuple = 0x22,
            ReflectionType_ValueArray = 0x23,
            ReflectionType_ValueTable = 0x24,
            ReflectionType_ValueMap = 0x25,
            ReflectionType_Variant = 0x26,
            ReflectionType_GenericFunction = 0x27,
            ReflectionType_WeakFunctionRef = 0x28,
            ReflectionType_ColorSequence = 0x29,
            ReflectionType_ColorSequenceKeypoint = 0x2a,
            ReflectionType_NumberRange = 0x2b,
            ReflectionType_NumberSequence = 0x2c,
            ReflectionType_NumberSequenceKeypoint = 0x2d,
            ReflectionType_InputObject = 0x2e,
            ReflectionType_Connection = 0x2f,
            ReflectionType_ContentId = 0x30,
            ReflectionType_DescribedBase = 0x31,
            ReflectionType_RefType = 0x32,
            ReflectionType_QFont = 0x33,
            ReflectionType_QDir = 0x34,
            ReflectionType_EventInstance = 0x35,
            ReflectionType_TweenInfo = 0x36,
            ReflectionType_DockWidgetPluginGuiInfo = 0x37,
            ReflectionType_PluginDrag = 0x38,
            ReflectionType_Random = 0x39,
            ReflectionType_PathWaypoint = 0x3a,
            ReflectionType_FloatCurveKey = 0x3b,
            ReflectionType_RotationCurveKey = 0x3c,
            ReflectionType_SharedString = 0x3d,
            ReflectionType_DateTime = 0x3e,
            ReflectionType_RaycastParams = 0x3f,
            ReflectionType_RaycastResult = 0x40,
            ReflectionType_OverlapParams = 0x41,
            ReflectionType_LazyTable = 0x42,
            ReflectionType_DebugTable = 0x43,
            ReflectionType_CatalogSearchParams = 0x44,
            ReflectionType_OptionalCoordinateFrame = 0x45,
            ReflectionType_CSGPropertyData = 0x46,
            ReflectionType_UniqueId = 0x47,
            ReflectionType_Font = 0x48,
            ReflectionType_Blackboard = 0x49,
            ReflectionType_Max = 0x4a
        };

        struct ClassDescriptor;
        struct Descriptor
        {
            enum ThreadSafety : uint32_t { Unset = 0x0, Unsafe = 0x1, ReadSafe = 0x3, LocalSafe = 0x7, Safe = 0xf };
            struct Attributes
            {
                bool isDeprecated;
                class RBX::Reflection::Descriptor* preferred;
                enum RBX::Reflection::Descriptor::ThreadSafety threadSafety;
            };
            void* vftable;
            std::string& name;
            struct RBX::Reflection::Descriptor::Attributes attributes;
        };

        struct Type : RBX::Reflection::Descriptor
        {
            std::string& tag;
            char _dd[0x8];
            enum RBX::Reflection::ReflectionType reflectionType;
            bool isFloat;
            bool isNumber;
            bool isEnum;
            bool isOptional;
        };

        struct Storage
        {
            const type_holder* holder;
            char data[64];
        };

        const struct Variant
        {
            const Type* _type;
            Storage value;
        };

        const struct Tuple
        {
            std::vector<RBX::Reflection::Variant, std::allocator<RBX::Reflection::Variant> > values;
        };

        struct TupleBase
        {
            RBX::Reflection::Tuple* _Ptr;
            std::_Ref_count_base* _Rep;
        };

        struct TupleShared : TupleBase {};

        struct EnumDescriptor : RBX::Reflection::Type
        {
            std::vector<void*> allItems;
            std::uint64_t enumCount;
            const char _0[0x60];
        };

        struct MemberDescriptor : RBX::Reflection::Descriptor
        {
            std::string& category;
            class RBX::Reflection::ClassDescriptor* owner;
            enum RBX::Security::Permissions permissions;
            int32_t _0;
        };

        struct EventDescriptor : RBX::Reflection::MemberDescriptor {};

        struct PropertyDescriptorVFT {};

        struct PropertyDescriptor : RBX::Reflection::MemberDescriptor
        {
        public:
            union {
                uint32_t bIsEditable;
                uint32_t bCanReplicate;
                uint32_t bCanXmlRead;
                uint32_t bCanXmlWrite;
                uint32_t bAlwaysClone;
                uint32_t bIsScriptable;
                uint32_t bIsPublic;
            } __bitfield;
            char _dd[0x8];
            RBX::Reflection::Type* type;
            bool bIsEnum;
            RBX::Security::Permissions scriptWriteAccess;

            bool IsScriptable() { return (this->__bitfield.bIsScriptable > 0x20) & 1; }

            void SetScriptable(const uint8_t bIsScriptable)
            {
                this->__bitfield.bIsScriptable = (this->__bitfield.bIsScriptable) ^ (bIsScriptable ? 33 : 32);
            }

            bool IsEditable() { return ((this->__bitfield.bIsEditable) & 1); }

            void SetEditable(const uint8_t bIsEditable)
            {
                this->__bitfield.bIsEditable = (this->__bitfield.bIsEditable) ^ ((~bIsEditable & 0xFF));
            }

            bool IsCanXmlRead() { return ((this->__bitfield.bCanXmlRead >> 3) & 1); }
            void SetCanXmlRead(const uint8_t bCanXmlRead)
            {
                this->__bitfield.bCanXmlRead = (this->__bitfield.bCanXmlRead) ^ ((~bCanXmlRead & 0xFF << 3));
            }

            bool IsCanXmlWrite() { return ((this->__bitfield.bCanXmlWrite >> 4) & 1); }
            void SetCanXmlWrite(const uint8_t bCanXmlWrite)
            {
                this->__bitfield.bCanXmlWrite = (this->__bitfield.bCanXmlWrite) ^ ((~bCanXmlWrite & 0xFF << 4));
            }

            bool IsPublic() { return ((this->__bitfield.bIsPublic >> 6) & 1); }
            void SetIsPublic(const uint8_t bIsPublic)
            {
                this->__bitfield.bIsPublic =
                    (this->__bitfield.bIsPublic) ^ static_cast<uint32_t>(~bIsPublic & 0xFF << 6);
            }

            bool IsCanReplicate() { return ((this->__bitfield.bCanReplicate >> 2) & 1); }
            void SetCanReplicate(const uint8_t bCanReplicate)
            {
                this->__bitfield.bCanReplicate = (this->__bitfield.bCanReplicate) ^ ((~bCanReplicate & 0xFF << 2));
            }

            bool IsAlwaysClone() { return ((this->__bitfield.bAlwaysClone) & 1); }
            void SetAlwaysClone(const uint8_t bAlwaysClone)
            {
                this->__bitfield.bAlwaysClone = (this->__bitfield.bAlwaysClone) ^ (~bAlwaysClone & 0xFF);
            }

            PropertyDescriptorVFT* GetVFT() { return static_cast<PropertyDescriptorVFT*>(this->vftable); }
        };

        struct EnumPropertyDescriptor : RBX::Reflection::PropertyDescriptor
        {
            RBX::Reflection::EnumDescriptor* enumDescriptor;
        };

        template<Concepts::_TypeConstraint<RBX::Reflection::MemberDescriptor> U>
        struct MemberDescriptorContainer
        {
            std::vector<U*> descriptors;
            const char _0[144];
        };

        struct ClassDescriptor : RBX::Reflection::Descriptor
        {
            char _dd[0x8];
            MemberDescriptorContainer<RBX::Reflection::PropertyDescriptor> propertyDescriptors;
            MemberDescriptorContainer<RBX::Reflection::EventDescriptor> eventDescriptors;
            void* boundFunctionDescription_Start;
            char _180[0x40];
            char _1c0[0x40];
            char _200[0x20];
            void* boundYieldFunctionDescription_Start;
            char _228[0x18];
            char _240[0x40];
            char _280[0x40];
            char _2c0[8];
            char _2c8[0x38];
            char _300[0x40];
            char _340[0x30];
            char _370[8];
            char _378[8];
            char _380[4];
            char _384[2];
            char _386[2];
            char _388[8];
            struct RBX::Reflection::ClassDescriptor* baseClass;
            char _398[8];
            char _3a0[8];
            char _3a8[0x18];
            char _3c0[0x20];
        };
    }

    struct Instance
    {
        void* vftable;
        std::weak_ptr<RBX::Instance> self;
        RBX::Reflection::ClassDescriptor* classDescriptor;
    };
}

struct signal_t;

struct signal_connection_t {
    char padding[16];
    int thread_idx; // 0x10
    int func_idx; //0x14
};

struct signal_data_t {
    uint64_t padding1;
    signal_t* root; //0x8
    uint64_t padding2[12];
    signal_connection_t* connection_data; //0x70
};

struct signal_t {
    uint64_t padding1[2];
    signal_t* next; //0x10
    uint64_t padding2;
    uint64_t state;
    uint64_t padding3;
    signal_data_t* signal_data; //0x30
};

struct connection_object {
    signal_t* signal;
    uint64_t state;
    uint64_t metatable;
    uint64_t root;
};

std::unordered_map<signal_t*, connection_object> connection_table;

int connection_blank(lua_State* rl) {
    return 0;
}

int disable_connection(lua_State* rl) {
    auto connection = (connection_object*)lua_touserdata(rl, 1);
    if (connection->signal->state != 0)
        connection->state = connection->signal->state;

    connection->signal->state = 0;
    return 0;
}

int enable_connection(lua_State* rl) {
    auto connection = (connection_object*)lua_touserdata(rl, 1);
    connection->signal->state = connection->state;
    return 0;
}

int disconnect_connection(lua_State* rl) {
    auto connection = (connection_object*)lua_touserdata(rl, 1);
    auto root = (signal_t*)connection->root;
    if ((uint64_t)root == (uint64_t)connection) {
        luaL_error(rl, "Failed to disconnect.");
    }

    while (root->next && root->next != connection->signal) {
        root = root->next;
    }

    if (!root->next) {
        luaL_error(rl, "Already disconnected.");
    }

    root->next = root->next->next;
    connection->signal->state = 0;
    return 0;
}

int connection_index(lua_State* rl) {
    std::string key = std::string(lua_tolstring(rl, 2, nullptr));
    auto connection = (connection_object*)lua_touserdata(rl, 1);
    uintptr_t connection2 = *(uintptr_t*)lua_touserdata(rl, 1);

    if (key == "Enabled" || key == "enabled") {
        lua_pushboolean(rl, !(connection->signal->state == 0));
        return 1;
    }

    if (key == "Function" || key == "function" || key == "Fire" || key == "fire" || key == "Defer" || key == "defer") {
        int signal_data = *(int*)&connection->signal->signal_data;
        if (signal_data && *(int*)&connection->signal->signal_data->connection_data) {
            int index = connection->signal->signal_data->connection_data->func_idx;
            lua_rawgeti(rl, LUA_REGISTRYINDEX, index);

            if (lua_type(rl, -1) != LUA_TFUNCTION)
                lua_pushcclosure(rl, connection_blank, 0, 0, 0);

            return 1;
        }

        lua_pushcclosure(rl, connection_blank, 0, 0, 0);
        return 1;
    }

    if (key == "LuaConnection") {
        int signal_data = *(int*)&connection->signal->signal_data;
        if (signal_data && *(int*)&connection->signal->signal_data->connection_data) {
            int index = connection->signal->signal_data->connection_data->func_idx;

            lua_rawgeti(rl, LUA_REGISTRYINDEX, index);
            auto func_tval = (TValue*)luaA_toobject(rl, -1);
            auto cl = (Closure*)func_tval->value.gc;
            bool lua = !cl->isC;

            lua_pop(rl, 1);
            lua_pushboolean(rl, lua);
            return 1;
        }

        lua_pushboolean(rl, false);
        return 1;
    }

    if (key == "ForeignState") {
        int signal_data = *(int*)&connection->signal->signal_data;
        if (signal_data && *(int*)&connection->signal->signal_data->connection_data) {
            int index = connection->signal->signal_data->connection_data->func_idx;

            lua_rawgeti(rl, LUA_REGISTRYINDEX, index);
            auto func_tval = (TValue*)luaA_toobject(rl, -1);
            auto cl = (Closure*)func_tval->value.gc;
            bool c = cl->isC;

            lua_pop(rl, 1);
            lua_pushboolean(rl, c);
            return 1;
        }

        lua_pushboolean(rl, false);
        return 1;
    }

    if (key == "Disconnect" || key == "disconnect") {
        lua_pushcclosure(rl, disconnect_connection, 0, 0, 0);
        return 1;
    }

    if (key == "Disable" || key == "disable") {
        lua_pushcclosure(rl, disable_connection, 0, 0, 0);
        return 1;
    }

    if (key == "Enable" || key == "enable") {
        lua_pushcclosure(rl, enable_connection, 0, 0, 0);
        return 1;
    }

    if (key == "Thread") {
        int signal_data = *(int*)&connection->signal->signal_data;
        if (signal_data && *(int*)&connection->signal->signal_data->connection_data) {
            int index = connection->signal->signal_data->connection_data->thread_idx;
            lua_rawgeti(rl, LUA_REGISTRYINDEX, index);

            if (lua_type(rl, -1) != LUA_TTHREAD)
                lua_pushthread(rl);

            return 1;
        }
    }

    luaL_error(rl, "Invalid idx");
    return 0;
}

int getconnections_handler(lua_State* rl) {
    auto signal = *(signal_t**)lua_touserdata(rl, 1);
    signal = signal->next;

    lua_createtable(rl, 0, 0);
    auto signal_root = signal;
    int index = 1;

    while (signal) {
        int func_idx = signal->signal_data->connection_data->func_idx;

        if (!connection_table.count(signal)) {
            connection_object new_connection;
            new_connection.signal = signal;
            new_connection.root = (uint64_t)signal_root;
            new_connection.state = signal->state;
            connection_table[signal] = new_connection;
        }

        auto connection = (connection_object*)lua_newuserdata(rl, sizeof(connection_object), 0);
        *connection = connection_table[signal];

        lua_createtable(rl, 0, 0);
        lua_pushcclosure(rl, connection_index, 0, 0, 0);
        lua_setfield(rl, -2, "__index");

        lua_pushstring(rl, "RealConnection");
        lua_setfield(rl, -2, "__type");
        lua_setmetatable(rl, -2);

        lua_rawseti(rl, -2, index++);
        signal = signal->next;
    }

    return 1;
}

namespace Instances {
    int getcallbackvalue(lua_State* L) {
        SetFunction("instances::getcallbackvalue");
        luaL_checktype(L, 1, LUA_TUSERDATA);
        luaL_checktype(L, 2, LUA_TSTRING);

        auto Instance = *(uintptr_t*)lua_touserdata(L, 1);

        int Atom;
        auto Property = lua_tostringatom(L, 2, &Atom);

        const auto KTable = (uintptr_t*)Offsets::KTable;
        auto Descriptor = KTable[Atom];
        if (!Descriptor)
            return 0;

        auto ClassDescriptor = *(uintptr_t*)(Instance + 0x18);
        const auto Prop = RBX::GetProperty(ClassDescriptor + 0x3b8, &Descriptor);
        if (!Prop)
            return 0;

        int Index = *(uint32_t*)(*(uintptr_t*)(*(uintptr_t*)(*(uintptr_t*)((Instance + *(uintptr_t*)(*Prop + 0x80)) + 0x18) + 0x38) + 0x28) + 0x14);
        lua_getref(L, Index);

        if (lua_isfunction(L, -1))
            return 1;

        lua_pop(L, 1);

        return 0;
    }

    int isscriptable(lua_State* L)
    {
        SetFunction("instances::isscriptable");
        luaL_checktype(L, 1, LUA_TUSERDATA);
        luaL_checktype(L, 2, LUA_TSTRING);

        const auto userData = lua_touserdata(L, 1);
        const auto InstancePtr = *reinterpret_cast<uintptr_t*>(userData);

        int Atom;
        auto Property = lua_tostringatom(L, 2, &Atom);

        const auto KTable = (uintptr_t*)Offsets::KTable;
        auto Descriptor = KTable[Atom];
        if (!Descriptor) {
            lua_pushnil(L);
            return 1;
        }

        auto ClassDescriptor = *(uintptr_t*)(InstancePtr + Offsets::Instance::ClassDescriptor);
        const auto Prop = RBX::GetProperty(ClassDescriptor + Offsets::Instance::PropertyDescriptor, &Descriptor);
        if (!Prop) {
            lua_pushboolean(L, false);
            return 1;
        }

        int cachedProperty = getCachedScriptableProperty(InstancePtr, Property);
        int cachedDefaultProperty = getCachedDefultScriptableProperty(Property);

        if (cachedProperty != -1) {
            lua_pushboolean(L, cachedProperty);
            return 1;
        }

        if (cachedDefaultProperty != -1) {
            lua_pushboolean(L, cachedDefaultProperty);
            return 1;
        }

        std::uint64_t scriptable = *reinterpret_cast<std::uint64_t*>(*Prop + 0x48);
        lua_pushboolean(L, scriptable & 0x20);

        return 1;
    }

    int setscriptable(lua_State* L)
    {
        SetFunction("instances::setscriptable");
        luaL_checktype(L, 1, LUA_TUSERDATA);
        luaL_checktype(L, 2, LUA_TSTRING);
        luaL_checktype(L, 3, LUA_TBOOLEAN);

        const auto userData = lua_touserdata(L, 1);
        const auto InstancePtr = *reinterpret_cast<uintptr_t*>(userData);
        const auto scriptableBool = lua_toboolean(L, 3);

        int Atom;
        auto Property = lua_tostringatom(L, 2, &Atom);

        const auto KTable = (uintptr_t*)Offsets::KTable;
        auto Descriptor = KTable[Atom];
        if (!Descriptor) {
            lua_pushnil(L);
            return 1;
        }

        auto ClassDescriptor = *(uintptr_t*)(InstancePtr + Offsets::Instance::ClassDescriptor);
        const auto Prop = RBX::GetProperty(ClassDescriptor + Offsets::Instance::PropertyDescriptor, &Descriptor);
        if (!Prop) {
            lua_pushboolean(L, false);
            return 1;
        }

        if (!findAndUpdateScriptAbleCache(InstancePtr, Property, scriptableBool))
            script_able_cache.push_back({ InstancePtr, Property, scriptableBool });

        std::uint64_t scriptable = *reinterpret_cast<std::uint64_t*>(*Prop + 0x48);
        bool wasScriptAble = scriptable & 0x20;

        addDefaultPropertyState(Property, scriptable & 0x20);

        *reinterpret_cast<std::uint64_t*>(*Prop + 0x48) = std::bitset< 32 >{ scriptable }.set(5, scriptableBool).to_ulong();
        lua_pushboolean(L, wasScriptAble);
        return 1;
    }

    int gethiddenproperty(lua_State* L) {
        SetFunction("instances::gethiddenproperty");
        
        luaL_checktype(L, 1, LUA_TUSERDATA);
        luaL_checktype(L, 2, LUA_TSTRING);
        std::string PropName = lua_tostring(L, 2);
        const auto rbxInstance = *reinterpret_cast<RBX::Instance**>(lua_touserdata(L, 1));
        
        RBX::Reflection::ReflectionType oldPropType = RBX::Reflection::ReflectionType_Void;
        bool isSharedString = false;

        for (const auto& Prop : rbxInstance->classDescriptor->propertyDescriptors.descriptors) {
            if (Prop->name == PropName) {
                if (Prop->type->reflectionType == RBX::Reflection::ReflectionType::ReflectionType_SharedString) {
                    oldPropType = RBX::Reflection::ReflectionType::ReflectionType_SharedString;
                    Prop->type->reflectionType = RBX::Reflection::ReflectionType::ReflectionType_String;
                    isSharedString = true;
                    break;
                }
                else if (Prop->type->reflectionType == RBX::Reflection::ReflectionType::ReflectionType_BinaryString) {
                    oldPropType = RBX::Reflection::ReflectionType::ReflectionType_BinaryString;
                    Prop->type->reflectionType = RBX::Reflection::ReflectionType::ReflectionType_String;
                    break;
                }
            }
        }

        lua_pushcclosure(L, isscriptable, 0, 0);
        lua_pushvalue(L, 1);
        lua_pushstring(L, PropName.c_str());
        lua_call(L, 2, 1);
        const auto was = lua_toboolean(L, -1);
        lua_pop(L, 1);

        lua_pushcclosure(L, setscriptable, 0, 0);
        lua_pushvalue(L, 1);
        lua_pushstring(L, PropName.c_str());
        lua_pushboolean(L, true);
        lua_pcall(L, 3, 1, 0);
        lua_pop(L, 1);

        if (isSharedString) {
            lua_getfield(L, 1, PropName.c_str());

            if (!lua_isnil(L, -1)) {
                lua_getmetatable(L, -1);
                if (lua_istable(L, -1)) {
                    lua_getfield(L, -1, "__tostring");
                    if (lua_isfunction(L, -1)) {
                        lua_pushvalue(L, -3);
                        lua_call(L, 1, 1);
                        lua_replace(L, -3);
                        lua_pop(L, 1);
                    }
                    else {
                        lua_pop(L, 1);
                        lua_pop(L, 1);
                        const char* str = lua_tostring(L, -1);
                        if (!str) {
                            lua_pop(L, 1);
                            lua_pushstring(L, "SharedString_Content");
                        }
                    }
                }
                else {
                    lua_pop(L, 1);
                    lua_pop(L, 1);
                    lua_pushstring(L, "SharedString_Content");
                }
            }
            else {
                lua_pop(L, 1);
                lua_pushstring(L, "SharedString_Content");
            }
        }
        else {
            lua_getfield(L, 1, PropName.c_str());
        }

        lua_pushboolean(L, !was);

        lua_pushcclosure(L, setscriptable, 0, 0);
        lua_pushvalue(L, 1);
        lua_pushstring(L, PropName.c_str());
        lua_pushboolean(L, was);
        lua_pcall(L, 3, 1, 0);
        lua_pop(L, 1);

        if (oldPropType != RBX::Reflection::ReflectionType_Void) {
            for (const auto& Prop : rbxInstance->classDescriptor->propertyDescriptors.descriptors) {
                if (Prop->name == PropName) {
                    Prop->type->reflectionType = oldPropType;
                    break;
                }
            }
        }

        return 2;
    }



    int sethiddenproperty(lua_State* L) {
        SetFunction("instances::sethiddenproperty");
        luaL_checktype(L, 1, LUA_TUSERDATA);
        luaL_checktype(L, 2, LUA_TSTRING);
        luaL_checkany(L, 3);

        std::string PropName = lua_tostring(L, 2);

        lua_pushcclosure(L, isscriptable, 0, 0);
        lua_pushvalue(L, 1);
        lua_pushstring(L, PropName.c_str());
        lua_call(L, 2, 1);

        const auto was = lua_toboolean(L, -1);
        lua_pop(L, 1);

        lua_pushcclosure(L, setscriptable, 0, 0);
        lua_pushvalue(L, 1);
        lua_pushstring(L, PropName.c_str());
        lua_pushboolean(L, true);
        lua_pcall(L, 3, 1, 0);
        lua_pop(L, 1);

        lua_pushvalue(L, 3);
        lua_setfield(L, 1, PropName.c_str());
        lua_pushboolean(L, !was);

        lua_pushcclosure(L, setscriptable, 0, 0);
        lua_pushvalue(L, 1);
        lua_pushstring(L, PropName.c_str());
        lua_pushboolean(L, was);
        lua_pcall(L, 3, 1, 0);
        lua_pop(L, 1);

        return 1;
    }


    int gethui(lua_State* L) {
        SetFunction("instances::gethui");
        lua_getglobal(L, "__hiddenui");

        return 1;
    }

    int getinstances(lua_State* L) {
        SetFunction("instances::getinstances");
        static auto lastWriteTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastWriteTime).count();
        if (elapsedTime < 5) // 5ms debounce
            return 0;

        lastWriteTime = currentTime;

        if (!L || !L->global || !L->global->allgcopages) {
            luaL_error(L, "Invalid Lua state or global object.");
            return 0;
        }

        struct CInstanceContext
        {
            lua_State* L;
            __int64 n;
        } Context = { L, 0 };

        lua_createtable(L, 0, 0);

        for (lua_Page* Page = L->global->allgcopages; Page;)
        {
            if (!Page) continue;

            lua_Page* Next{ Page->listnext };

            luaM_visitpage(Page, &Context, [](void* Context, lua_Page* Page, GCObject* Gco) -> bool {
                auto GcContext = (CInstanceContext*)Context;

                if (!Gco) return true;

                const auto Type = Gco->gch.tt;

                if (Type == LUA_TUSERDATA)
                {
                    lua_State* L = GcContext->L;

                    if (L->top == nullptr || (L->top - L->stack) >= L->stacksize) {
                        luaL_error(L, "Invalid Lua stack access.");
                        return false;
                    }

                    TValue* top = L->top;
                    top->value.p = reinterpret_cast<void*>(Gco);
                    top->tt = Type;
                    L->top++;

                    if (!strcmp(luaL_typename(L, -1), "Instance"))
                    {
                        GcContext->n++;
                        lua_rawseti(GcContext->L, -2, GcContext->n);
                    }
                    else
                        lua_pop(L, 1);
                }

                return true;
            });

            if (!Page->listnext) break;
            Page = Next;
        }

        return 1;
    }

    int getnilinstances(lua_State* L) {
        SetFunction("instances::getnilinstances");
        static auto lastWriteTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastWriteTime).count();
        if (elapsedTime < 5) // 5ms debounce
            std::this_thread::sleep_for(std::chrono::milliseconds(5));

        lastWriteTime = currentTime;

        if (!L || !L->global || !L->global->allgcopages) {
            luaL_error(L, "Invalid Lua state or global object.");
            return 0;
        }

        struct CInstanceContext {
            lua_State* L;
            __int64 n;
        };

        CInstanceContext Context = { L, 0 };

        lua_createtable(L, 0, 0);

        for (lua_Page* Page = L->global->allgcopages; Page != nullptr; Page = Page->listnext) {
            if (!Page) continue;

            luaM_visitpage(Page, &Context, [](void* Context, lua_Page* Page, GCObject* Gco) -> bool {
                auto GcContext = (CInstanceContext*)Context;

                if (!Gco) return true;

                const auto Type = Gco->gch.tt;

                if (Type == LUA_TUSERDATA) {
                    lua_State* L = GcContext->L;
                    if (L->top == nullptr || (L->top - L->stack) >= L->stacksize) {
                        luaL_error(L, "Invalid Lua stack access.");
                        return false;
                    }

                    TValue* top = L->top;
                    top->value.p = reinterpret_cast<void*>(Gco);
                    top->tt = Type;
                    L->top++;

                    if (!strcmp(luaL_typename(L, -1), "Instance")) {
                        lua_getfield(L, -1, "Parent");

                        const auto NullParent = lua_isnoneornil(L, -1);
                        if (NullParent) {
                            lua_pop(L, 1);
                            GcContext->n++;
                            lua_rawseti(L, -2, GcContext->n);
                        }
                        else {
                            lua_pop(L, 2);
                        }
                    }
                    else {
                        lua_pop(L, 1);
                    }
                }

                return true;
            });

            if (!Page->listnext) break;
        }

        return 1;
    }

    int fireclickdetector(lua_State* L) {
        SetFunction("instances::fireclickdetector");
        luaL_checktype(L, 1, LUA_TUSERDATA);

        std::string clickOption = lua_isstring(L, 3) ? lua_tostring(L, 3) : "";

        if (strcmp(luaL_typename(L, 1), "Instance") != 0)
        {
            luaL_typeerror(L, 1, "Instance");
            return 0;
        }

        const auto clickDetector = *reinterpret_cast<uintptr_t*>(lua_touserdata(L, 1));

        float distance = 0.0;

        if (lua_isnumber(L, 2))
            distance = (float)lua_tonumber(L, 2);

        lua_getglobal(L, "game");
        lua_getfield(L, -1, "GetService");
        lua_insert(L, -2);
        lua_pushstring(L, "Players");
        lua_pcall(L, 2, 1, 0);

        lua_getfield(L, -1, "LocalPlayer");

        const auto localPlayer = *reinterpret_cast<uintptr_t*>(lua_touserdata(L, -1));

        std::transform(clickOption.begin(), clickOption.end(), clickOption.begin(), ::tolower);

        if (clickOption == "rightmouseclick")
            RBX::FireRightMouseClick(clickDetector, distance, localPlayer);
        else if (clickOption == "mousehoverenter")
            RBX::FireMouseHoverEnter(clickDetector, localPlayer);
        else if (clickOption == "mousehoverleave")
            RBX::FireMouseHoverLeave(clickDetector, localPlayer);
        else
            RBX::FireMouseClick(clickDetector, distance, localPlayer);
        return 0;
    }

    int getscripts(lua_State* L) {
        SetFunction("instances::getscripts");

        struct instancecontext {
            lua_State* L;
            __int64 n;
        };

        instancecontext Context = { L, 0 };

        lua_createtable(L, 0, 0);

        const auto originalGCThreshold = L->global->GCthreshold;
        L->global->GCthreshold = SIZE_MAX;

        luaM_visitgco(L, &Context, [](void* ctx, lua_Page* page, GCObject* gco) -> bool {
            auto context = static_cast<instancecontext*>(ctx);
            lua_State* L = context->L;

            if (isdead(L->global, gco))
                return false;

            if (gco->gch.tt == LUA_TUSERDATA) {
                TValue* top = L->top;
                top->value.p = reinterpret_cast<void*>(gco);
                top->tt = LUA_TUSERDATA;
                L->top++;

                if (strcmp(luaL_typename(L, -1), "Instance") == 0) {
                    lua_getfield(L, -1, "ClassName");
                    const char* className = lua_tolstring(L, -1, 0);

                    if (className && (
                        strcmp(className, "LocalScript") == 0 ||
                        strcmp(className, "ModuleScript") == 0 ||
                        strcmp(className, "Script") == 0))
                    {
                        lua_pop(L, 1);

                        lua_getfield(L, -1, "Parent");
                        if (lua_isnil(L, -1)) {
                            lua_pop(L, 1);

                            context->n++;
                            lua_rawseti(L, -2, context->n);
                        }
                        else {
                            lua_pop(L, 2);
                        }
                    }
                    else {
                        lua_pop(L, 2);
                    }
                }
                else {
                    lua_pop(L, 1);
                }
            }

            return true;
            });

        L->global->GCthreshold = originalGCThreshold;

        return 1;
    }

    int getrunningscripts(lua_State* L) {
        SetFunction("instances::getrunningscripts");

        int threadCount = 0;
        lua_pushvalue(L, LUA_REGISTRYINDEX);
        lua_pushnil(L);
        while (lua_next(L, -2) && threadCount < 1000) {
            if (lua_isthread(L, -1)) {
                threadCount++;
            }
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        std::unordered_map<uintptr_t, bool> map;
        map.reserve(threadCount > 0 ? threadCount * 2 : 64);
        
        lua_pushvalue(L, LUA_REGISTRYINDEX);
        
        lua_newtable(L);
        
        lua_pushnil(L);
        
        unsigned int c = 0u;
        while (lua_next(L, -3))
        {
            if (lua_isthread(L, -1))
            {
                const auto thread = lua_tothread(L, -1);
                
                if (thread)
                {
                    if (const auto script_ptr = reinterpret_cast<std::uintptr_t>(thread->userdata) + 0x50; *reinterpret_cast<std::uintptr_t*>(script_ptr))
                    {
                        if (map.find(*(uintptr_t*)script_ptr) == map.end())
                        {
                            map.insert({ *(uintptr_t*)script_ptr, true });
                            
                            RBX::PushInstance(L, (void*)script_ptr);
                            
                            lua_rawseti(L, -4, ++c);
                        }
                    }
                }
            }
            
            lua_pop(L, 1);
        }
        
        return 1;
    }

    int getloadedmodules(lua_State* L)
    {
        SetFunction("instances::getloadedmodules");

        lua_newtable(L);

        typedef struct {
            lua_State* pLua;
            int itemsFound;
            std::map< uintptr_t, bool > map;
        } GCOContext;

        auto gcCtx = GCOContext{ L, 0 };

        const auto ullOldThreshold = L->global->GCthreshold;
        L->global->GCthreshold = SIZE_MAX;

        luaM_visitgco(L, &gcCtx, [](void* ctx, lua_Page* pPage, GCObject* pGcObj) -> bool {
            const auto pCtx = static_cast<GCOContext*>(ctx);
            const auto ctxL = pCtx->pLua;

            if (isdead(ctxL->global, pGcObj))
                return false;

            if (const auto gcObjType = pGcObj->gch.tt;
                gcObjType == LUA_TFUNCTION) {
                ctxL->top->value.gc = pGcObj;
                ctxL->top->tt = gcObjType;
                ctxL->top++;

                lua_getfenv(ctxL, -1);

                if (!lua_isnil(ctxL, -1)) {
                    lua_getfield(ctxL, -1, "script");

                    if (!lua_isnil(ctxL, -1)) {
                        uintptr_t script_addr = *(uintptr_t*)lua_touserdata(ctxL, -1);
                        if (!CheckMemory(script_addr))
                            return false;

                        uintptr_t classDescriptor = *(uintptr_t*)(script_addr + 0x18);
                        if (!CheckMemory(classDescriptor))
                            return false;

                        std::string class_name = **(std::string**)(classDescriptor + 0x8);

                        if (pCtx->map.find(script_addr) == pCtx->map.end() && class_name == "ModuleScript") {
                            pCtx->map.insert({ script_addr, true });
                            lua_rawseti(ctxL, -4, ++pCtx->itemsFound);
                        }
                        else {
                            lua_pop(ctxL, 1);
                        }
                    }
                    else {
                        lua_pop(ctxL, 1);
                    }
                }

                lua_pop(ctxL, 2);
            }
            return false;
        });

        L->global->GCthreshold = ullOldThreshold;

        return 1;
    }

    int getconnections(lua_State* L)
    {
        SetFunction("instances::getconnections");

        luaL_checktype(L, 1, LUA_TUSERDATA);

        lua_getfield(L, 1, "Connect");
        if (!lua_isfunction(L, -1)) {
            luaL_error(L, "Signal does not have 'Connect' method");
        }

        lua_pushvalue(L, 1);
        lua_pushcfunction(L, connection_blank, nullptr);

        if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
            const char* err = lua_tostring(L, -1);
            luaL_error(L, "Error calling 'Connect': %s", err ? err : "unknown error");
        }

        if (!lua_istable(L, -1) && !lua_isuserdata(L, -1)) {
            luaL_error(L, "Connect did not return a valid connection object");
        }

        lua_pushvalue(L, -1);

        lua_pushcfunction(L, getconnections_handler, nullptr);
        lua_pushvalue(L, -2);

        if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
            const char* err = lua_tostring(L, -1);
            luaL_error(L, "Error calling 'getconnections': %s", err ? err : "unknown error");
        }

        lua_getfield(L, -3, "Disconnect");
        if (lua_isfunction(L, -1)) {
            lua_pushvalue(L, -4);
            if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
                const char* err = lua_tostring(L, -1);
                luaL_error(L, "Error calling 'Disconnect': %s", err ? err : "unknown error");
            }
        }
        else {
            lua_pop(L, 1);
        }

        lua_replace(L, 1);
        lua_settop(L, 1);
        return 1;
    }

    int firesignal(lua_State* L)
    {
        SetFunction("instances::firesignal");
        int nargs = lua_gettop(L);
        lua_pushcfunction(L, getconnections, "");
        lua_pushvalue(L, 1);
        lua_pcall(L, 1, 1, 0);
        lua_pushnil(L);

        while (lua_next(L, -2)) {
            lua_getfield(L, -1, "Function");
            for (int i = 0; i < nargs - 1; i++)
                lua_pushvalue(L, i + 2);
            lua_pcall(L, nargs - 1, 0, 0);
            lua_pop(L, 1);
        }
        return 0;
    }

    int firetouchinterest(lua_State* L) {
        SetFunction("instances::firetouchinterest");
        luaL_checktype(L, 1, LUA_TUSERDATA);
        luaL_checktype(L, 2, LUA_TUSERDATA);
        luaL_checktype(L, 3, LUA_TNUMBER);

        int Toggle = lua_tonumber(L, 3);

        const auto Instance = *reinterpret_cast<uintptr_t*>(lua_touserdata(L, 1));
        const auto Instance2 = *reinterpret_cast<uintptr_t*>(lua_touserdata(L, 2));

        const auto Prim1 = *reinterpret_cast<uintptr_t*>(Instance + 0x170);
        if (!Prim1)
            luaL_argerror(L, 1, "Failed to get Primitive 1");

        const auto Prim2 = *reinterpret_cast<uintptr_t*>(Instance2 + 0x170);

        if (!Prim2)
            luaL_argerror(L, 2, "Failed to get Primitive 2");
        const auto Overlap = *reinterpret_cast<uintptr_t*>(Prim1 + 0x1D0);

        RBX::FireTouchInterest(Overlap, Prim1, Prim2, Toggle, true);

        return 0;
    }

    int fireproximityprompt(lua_State* L) {
        luaL_checktype(L, 1, LUA_TUSERDATA);

        uintptr_t prompt = *(uintptr_t*)(lua_topointer(L, 1));
        RBX::FireProximityPrompt(prompt);
        return 0;
    };

    using TRaiseEventInvocation = void(__fastcall*)(uintptr_t*, uintptr_t*, std::vector<RBX::Reflection::Variant>*, RBX::Reflection::EventSource::RemoteEventInvocationTargetOptions*);
    inline auto RaiseEventInvocation = (TRaiseEventInvocation)Offsets::RaiseEventInvocation;

    using TGetValues = std::shared_ptr<RBX::Reflection::Tuple>(__fastcall*)(std::shared_ptr<RBX::Reflection::Tuple>*, lua_State*);
    inline auto GetValues = (TGetValues)Offsets::GetValues;

    int replicatesignal(lua_State* L) {
        SetFunction("instances::replicatesignal");

        if (strcmp(luaL_typename(L, 1), "RBXScriptSignal") != 0)
        {
            luaL_typeerror(L, 1, "RBXScriptSignal");
            return 0;
        }

        SetFunction("instances::replicatesignal2");

        uintptr_t UserData = *reinterpret_cast<uintptr_t*>(lua_touserdata(L, 1));
        SetFunction("instances::replicatesignal3");
        uintptr_t Descriptor = *(uintptr_t*)(UserData);
        SetFunction("instances::replicatesignal4");
        uintptr_t Instance = *(uintptr_t*)(UserData + 0x18);
        SetFunction("instances::replicatesignal5");

        *(BYTE*)(Offsets::LockViolationInstanceCrash) = FALSE;
        SetFunction("instances::replicatesignal6");

        RBX::Reflection::EventSource::RemoteEventInvocationTargetOptions Options;
        Options.target = 0;
        Options.isExcludeTarget = RBX::Reflection::EventSource::OnlyTarget;
        SetFunction("instances::replicatesignal7");

        std::vector<RBX::Reflection::Variant> Arguments;
        SetFunction("instances::replicatesignal7.1");

        std::shared_ptr<RBX::Reflection::Tuple> Tuple;
        SetFunction("instances::replicatesignal7.2");
        GetValues(&Tuple, L);
        SetFunction("instances::replicatesignal8\n");

        int c = 1;
        for (auto it = Tuple->values.begin() + 1; it != Tuple->values.end(); ++it) {
            SetFunction("instances::replicatesignal8.1");
            const auto& variant = *it;
            SetFunction("instances::replicatesignal8.2");
            if (!variant._type) continue;
            SetFunction("instances::replicatesignal8.3");
            if (!variant._type->reflectionType) continue;
            SetFunction("instances::replicatesignal8.4");

            Arguments.push_back(variant);
            SetFunction("instances::replicatesignal8.5");
            c++;
            SetFunction("instances::replicatesignal8.6");
        }
        SetFunction("\ninstances::replicatesignal9");

        RaiseEventInvocation(reinterpret_cast<uintptr_t*>(Instance), reinterpret_cast<uintptr_t*>(Descriptor), &Arguments, &Options);
        SetFunction("instances::replicatesignal10");

        return 0;
    }
}
#pragma once
#include <cstdint>
#include <cstdarg>
#include <functional> 

struct XMLObject;
struct lua_State;
using guid_t = uint64_t;


#define lua_setglobal(L,s)	lua_setfield(L, LUA_GLOBALSINDEX, (s))
#define lua_pushcfunction(L, f) lua_pushcclosure(L, f, 0);

#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)

namespace Console {
    enum CVarFlags : uint32_t {
        CVarFlags_ReadOnly = 0x4,
        CVarFlags_CheckTaint = 0x8,
        CVarFlags_HideFromUser = 0x40,
        CVarFlags_ReadOnlyForUser = 0x100,

    };

    struct CVar {
        using Handler_t = int(*)(CVar* cvar, const char* prevVal, const char* newVal, void* userData);

        uint32_t hash;
        uint32_t gap4[4];
        const char* name;
        uint32_t field18;
        CVarFlags flags;
        uint32_t field20;
        uint32_t field24;
        const char* vStr;
        uint32_t field2C[5];
        uint32_t vBool;
        uint32_t gap44[9];
        Handler_t handler;
        void* userData;
    };
    static_assert(sizeof(CVar) == 0x70);

    inline CVar* RegisterCVar(const char* name, const char* desc, unsigned flags, const char* defaultVal, CVar::Handler_t callback, int a6, int a7, int a8, int a9) { return ((decltype(&RegisterCVar))0x00767FC0)(name, desc, flags, defaultVal, callback, a6, a7, a8, a9); };
    inline CVar* GetCVar(const char* name) { return ((decltype(&GetCVar))0x00767460)(name); }
    inline CVar* FindCVar(const char* name) { return ((decltype(&FindCVar))0x00767440)(name); }
    inline char SetCVarValue(CVar* self, const char* value, int a3, int a4, int a5, int a6)
    {
        return (((char(__thiscall*)(CVar*, const char*, int, int, int, int))0x007668C0))(self, value, a3, a4, a5, a6);
    }
}

namespace RCString {
    inline uint32_t __stdcall hash(const char* str) { return ((decltype(&hash))0x0076F640)(str); }
}

 
// XML
struct __declspec(novtable) XMLObject {
    uint32_t gap0[0x38 / 4];

    inline XMLObject(int a1, const char* parentName) { ((XMLObject * (__thiscall*)(XMLObject*, int, const char*))0x00814AD0)(this, a1, parentName); }
    inline void setValue(const char* key, const char* value) { ((void(__thiscall*)(XMLObject*, const char*, const char*))0x814C40)(this, key, value); }
};

using lua_CFunction = int(*)(lua_State*);

inline lua_State* GetLuaState() { return ((decltype(&GetLuaState))0x00817DB0)(); }

typedef struct luaL_Reg {
    const char* name;
    lua_CFunction func;
} luaL_Reg;


inline void lua_createtable(lua_State* L, int narr, int nrec) { return ((decltype(&lua_createtable))0x0084E6E0)(L, narr, nrec); }
inline void lua_setfield(lua_State* L, int idx, const char* str) { return ((decltype(&lua_setfield))0x0084E900)(L, idx, str); }
inline void lua_pushcclosure(lua_State* L, lua_CFunction func, int c) { return ((decltype(&lua_pushcclosure))0x0084E400)(L, func, c); }


inline bool IsInWorld() { return *(char*)0x00BD0792; }

// FrameScript
namespace FrameScript {
    struct Event {
        uint32_t hash;
        uint32_t gap4[4];
        const char* name;
        uint32_t gap18[12];
        uint32_t field48;
        uint32_t field4C;
        uint32_t field50;
    };

    struct EventList {
        size_t reserve;
        size_t size;
        Event** buf;
    };

    struct UnkContainer;

    inline UnkContainer* GetUnkContainer() { return (UnkContainer*)0x00D3F7A8; }
    inline Event* __fastcall FindEvent(UnkContainer* This, void* edx, const char* eventName) { return ((decltype(&FindEvent))0x004BC410)(This, edx, eventName); }
    inline EventList* GetEventList() { return (EventList*)0x00D3F7D0; }
    inline void FireEvent_inner(int eventId, lua_State* L, int nargs) { return ((decltype(&FireEvent_inner))0x0081AA00)(eventId, L, nargs); };
    inline void vFireEvent(int eventId, const char* format, va_list args) { return ((decltype(&vFireEvent))0x0081AC90)(eventId, format, args); }

    inline int GetEventIdByName(const char* eventName)
    {
        EventList* eventList = GetEventList();
        if (eventList->size == 0)
            return -1;

        uint32_t hash = RCString::hash(eventName);
        for (size_t i = 0; i < eventList->size; i++) {
            Event* event = eventList->buf[i];
            if (event && event->hash == hash && (event->name == eventName || (strcmp(event->name, eventName) == 0)))
                return i;
        }
        return -1;
    }

    inline const char* GetEventNameById(unsigned idx)
    {
        EventList* eventList = GetEventList();
        if (eventList->size == 0 || eventList->size < idx)
            return NULL;

        Event* event = eventList->buf[idx];
        return event ? event->name : NULL;
    }

    inline void FireEvent(const char* eventName, const char* format, ...)
    {
        int eventId = GetEventIdByName(eventName);
        if (eventId == -1) return;

        va_list args;
        va_start(args, format);
        vFireEvent(eventId, format, args);
    }
}

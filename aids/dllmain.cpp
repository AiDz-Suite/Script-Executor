#include "communication/communication.hpp"
#include "il2cpp/api.h"
#include "minhook/MinHook.h"

#include <Windows.h>
#include <thread>
#include <string>
#include <iostream>
#include <unordered_map>

extern "C"
{
#include "lua/lapi.h"
#include "lua/lobject.h"
#include "lua/lstate.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"
#include "lua/lundump.h"
#include "lua/lfunc.h"
}

const std::uintptr_t il2cpp_base = reinterpret_cast<std::uintptr_t>(GetModuleHandleA("GameAssembly.dll"));

template<typename T>
bool set_singleton(Il2CppClass* owner_class, const char* field_name, T &to_set)
{
    FieldInfo* field = il2cpp_class_get_field_from_name(owner_class, field_name);
    il2cpp_field_static_get_value(field, reinterpret_cast<void*>(&to_set));

    return to_set != 0;
}

// Singletons
Il2CppObject* instance_manager = nullptr;

// Classes

Il2CppClass* localscript_class = nullptr;
Il2CppClass* script_class = nullptr;
Il2CppClass* scriptmanager_class = nullptr;
Il2CppClass* instance_manager_class = nullptr;

// Methods

using scriptmanager_update_t = Il2CppObject*(*)(Il2CppObject* script_manager);
scriptmanager_update_t scriptmanager_update = nullptr;
scriptmanager_update_t hooked_scriptmanager_update = nullptr;

using scriptmanager_log_t = void(*)(Il2CppObject* script_manager, Il2CppObject* str_message);
scriptmanager_log_t scriptmanager_log = nullptr;

using script_loadscript_t = void(*)(Il2CppObject* script, Il2CppObject* str_code);
script_loadscript_t script_loadscript = nullptr;

using localscript_ctor_t = void(*)(Il2CppObject* local_script);
localscript_ctor_t localscript_ctor = nullptr;

using localscript_registertable_t = void(*)(Il2CppObject* local_script);
localscript_registertable_t localscript_registertable = nullptr;

using string_createstring_t = Il2CppObject*(*)(void* ignore, const char* val, std::size_t start, std::size_t len);
string_createstring_t string_createstring = nullptr;

using get_gameobject_t = Il2CppObject*(*)(Il2CppObject* component);
get_gameobject_t get_gameobject = nullptr;

using gameobject_setactive_t = void(*)(Il2CppObject* gameobject, bool active);
gameobject_setactive_t gameobject_setactive = nullptr;

// Properties

std::uint32_t dynamic_name_off = 0;
std::uint32_t script_env_off = 0;
std::uint32_t luabase_interpreter_off = 0;
std::uint32_t luabase_reference_off = 0;
std::uint32_t lua_luastate_off = 0;
std::uint32_t instancemanager_debugwindow_off = 0;
std::uint32_t debugwindow_navigationbutton_off = 0;

// Il2Cpp strings are NOT System.String strings.
Il2CppObject* make_string(const std::string& str)
{
    return string_createstring(nullptr, str.c_str(), 0, str.length());
}

// Exploit

BOOL WINAPI ctrl_handler(DWORD ctrl_method)
{
    if (ctrl_method == CTRL_C_EVENT)
    {
        std::printf("CTRL+C RECEIVED!\n");
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        return true;
    }

    return false;
}

void make_console()
{
    FILE* F;
    AllocConsole();
    freopen_s(&F, "CONOUT$", "w", stdout);
    freopen_s(&F, "CONIN$", "r", stdin);
    freopen_s(&F, "CONOUT$", "w", stderr);
    SetConsoleTitleA("AiDz | Executor");
    SetConsoleCtrlHandler(ctrl_handler, TRUE);
}

void set_debug_visible(bool visible = true)
{
    if (!instance_manager)
        return;
    Il2CppObject* debugwindow = *reinterpret_cast<Il2CppObject**>(reinterpret_cast<std::uintptr_t>(instance_manager) + instancemanager_debugwindow_off);
    if (!debugwindow)
        return;
    Il2CppObject* navigationbutton = *reinterpret_cast<Il2CppObject**>(reinterpret_cast<std::uintptr_t>(debugwindow) + debugwindow_navigationbutton_off);
    if (!navigationbutton)
        return;
    Il2CppObject* gameobject = get_gameobject(navigationbutton);
    if (!gameobject)
        return;
    gameobject_setactive(gameobject, visible);

    std::uintptr_t dogshit_check = il2cpp_base + 0x026B5BD;
    DWORD old_protection = 0;

    VirtualProtect(reinterpret_cast<void*>(dogshit_check), 2, PAGE_EXECUTE_READWRITE, &old_protection);
    *reinterpret_cast<std::uint16_t*>(dogshit_check) = 0x9090;
    VirtualProtect(reinterpret_cast<void*>(dogshit_check), 2, old_protection, &old_protection);
}

std::string rainbow_text(const std::string& text)
{
    srand(time(0));
    std::string output{};
    std::vector<std::string> colors = { "#e81416", "#ffa500", "#faeb36", "#79c314", "#487de7", "#70369d"};

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        std::string& selected_color = colors[i % colors.size()];
        output += "<color=" + selected_color + ">" + text[i] + "</color>";
    }

    return output;
}

int index_metamethod(lua_State* state)
{
    int args = lua_gettop(state);
    if (args < 2) // table, index
        return 0;

    lua_pushglobaltable(state);
    lua_pushvalue(state, 2);
    lua_rawget(state, -2); // check if idx is in global environment

    // Return global if found
    if (!lua_isnil(state, -1))
        return 1;

    lua_settop(state, args); // wipe stack back to start of func
    lua_pushvalue(state, lua_upvalueindex(1)); // get old metatable
    if (lua_isnil(state, -1)) // if it doesn't exist no point in running anymore code
        return 0;

    lua_getfield(state, -1, "__index"); // get index metamethod

    if (lua_isfunction(state, -1))
    { // call metamethod function
        int old = lua_gettop(state);
        lua_pushvalue(state, 2);
        lua_pcall(state, 2, LUA_MULTRET, 0);
        return lua_gettop(state) - old;
    }
    else if (lua_istable(state, -1))
    { // return from metamethod table
        lua_pushvalue(state, 1);
        lua_gettable(state, -2);
        return 1;
    }

    // didn't find it in global, and there's no valid metamethod to search for more
    return 0;
}

// Returns registry environment
int getreg(lua_State* state)
{
    lua_pushvalue(state, LUA_REGISTRYINDEX);
    return 1;
}

// Returns global environment
int getgenv(lua_State* state)
{
    lua_pushglobaltable(state);
    return 1;
}

int debug_unlock(lua_State* state)
{
    if (lua_gettop(state) != 1)
        return 0;

    set_debug_visible(lua_toboolean(state, 1));
    return 0;
}

// Create a script VIA Native API.
void run_lua_script(const std::string& script)
{
    Il2CppObject* cheat_script = il2cpp_object_new(localscript_class);
    localscript_ctor(cheat_script);

    Il2CppObject* exploit_name = make_string("AiDz");
    *reinterpret_cast<Il2CppObject**>(reinterpret_cast<std::uintptr_t>(cheat_script) + dynamic_name_off) = exploit_name;

    Il2CppObject* exploit_script = make_string(script);
    script_loadscript(cheat_script, exploit_script);

    localscript_registertable(cheat_script);

    Il2CppObject* nlua_luatable = *reinterpret_cast<Il2CppObject**>(reinterpret_cast<std::uintptr_t>(cheat_script) + script_env_off);
    Il2CppObject* nlua_lua = *reinterpret_cast<Il2CppObject**>(reinterpret_cast<std::uintptr_t>(nlua_luatable) + luabase_interpreter_off);
    lua_State* state = *reinterpret_cast<lua_State**>(reinterpret_cast<std::uintptr_t>(nlua_lua) + lua_luastate_off);

    int script_environment_idx = *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(nlua_luatable) + luabase_reference_off);
    int original_top = lua_gettop(state);

    std::printf("Lua State: 0x%p\n\n\n", state);
    std::printf("script environment index: %d\n", script_environment_idx);

    for (int i = 1; i <= original_top; ++i)
    {
        std::printf("%s\n", lua_typename(state, lua_type(state, i)));
    }

    lua_pushglobaltable(state);
    lua_pushcfunction(state, getreg);
    lua_setfield(state, -2, "getreg");

    lua_pushcfunction(state, getgenv);
    lua_setfield(state, -2, "getgenv");

    lua_getfield(state, -1, "debug");
    lua_pushcfunction(state, debug_unlock);
    lua_setfield(state, -2, "unlock");

    // Elevate environment to index into global table
    lua_rawgeti(state, LUA_REGISTRYINDEX, script_environment_idx);
    lua_newtable(state);
    lua_getmetatable(state, -2);
    lua_pushcclosure(state, index_metamethod, 1);
    lua_setfield(state, -2, "__index");
    lua_setmetatable(state, -2);

    lua_settop(state, original_top);
}

Il2CppObject* scriptmanager_update_hook(Il2CppObject* script_manager)
{
    static bool init_ran = false;

    if (!init_ran)
    {
        init_ran = true;
        std::string disclaimer = "<color=#ff0000>[AiDz]: Brickplanet has been PWNED by AiDz menu.</color>";
        scriptmanager_log(script_manager, make_string(disclaimer));
    }

    {
        std::lock_guard my_guard{ communication.pending_scripts_mutex };
        
        while (!communication.pending_scripts.empty())
        {
            std::string script = communication.pending_scripts.top();
            communication.pending_scripts.pop();

            std::printf("running script:\n%s\n", script.c_str());
            run_lua_script(script);
        }
    }

    return scriptmanager_update(script_manager);
}

void setup_hooks()
{
    MH_Initialize();

    // Script Hook for execution
    hooked_scriptmanager_update = scriptmanager_update;
    MH_CreateHook(hooked_scriptmanager_update, &scriptmanager_update_hook, reinterpret_cast<LPVOID*>(&scriptmanager_update));
    MH_EnableHook(hooked_scriptmanager_update);
}

// Automatically updates entire cheat with reflection
bool update_chair()
{
    Il2CppDomain* domain = il2cpp_domain_get();

    if (domain == nullptr)
        return false;

    // Attach to domain
    Il2CppThread* current_thread = il2cpp_thread_attach(domain);

    const Il2CppAssembly* assembly = il2cpp_domain_assembly_open(domain, "Assembly-CSharp");
    const Il2CppAssembly* mscorlib = il2cpp_domain_assembly_open(domain, "mscorlib");
    const Il2CppAssembly* unityassembly = il2cpp_domain_assembly_open(domain, "UnityEngine.CoreModule");

    if (assembly == nullptr || mscorlib == nullptr)
        return false;
    std::printf("Loaded assemblies!\n");

    const Il2CppImage* image = il2cpp_assembly_get_image(assembly);
    const Il2CppImage* mscorlib_img = il2cpp_assembly_get_image(mscorlib);
    const Il2CppImage* unityengine = il2cpp_assembly_get_image(unityassembly);

    if (image == nullptr || mscorlib_img == nullptr)
        return false;

    script_class = il2cpp_class_from_name(image, "Brickplanet.Scripting", "Script");
    localscript_class = il2cpp_class_from_name(image, "Brickplanet.Scripting", "LocalScript");
    scriptmanager_class = il2cpp_class_from_name(image, "Brickplanet.Scripting", "ScriptManager");
    instance_manager_class = il2cpp_class_from_name(image, "Brickplanet.Client", "InstanceManager");

    Il2CppClass* dynamic_class = il2cpp_class_from_name(image, "Brickplanet.Scripting", "Dynamic");
    Il2CppClass* lua_class = il2cpp_class_from_name(image, "NLua", "Lua");
    Il2CppClass* luabase_class = il2cpp_class_from_name(image, "NLua", "LuaBase");
    Il2CppClass* string_class = il2cpp_class_from_name(mscorlib_img, "System", "String");
    Il2CppClass* debugwindow_class = il2cpp_class_from_name(image, "Brickplanet.Client.UI", "DebugWindow");
    Il2CppClass* component_class = il2cpp_class_from_name(unityengine, "UnityEngine", "Component");
    Il2CppClass* gameobject_class = il2cpp_class_from_name(unityengine, "UnityEngine", "GameObject");

    if (localscript_class == nullptr || scriptmanager_class == nullptr || string_class == nullptr || script_class == nullptr || dynamic_class == nullptr || lua_class == nullptr 
        || luabase_class == nullptr || instance_manager_class == nullptr || debugwindow_class == nullptr || component_class == nullptr || gameobject_class == nullptr)
        return false;
    std::printf("Loaded classes!\n");

    FieldInfo* Dynamic_Name = il2cpp_class_get_field_from_name(dynamic_class, "Name");
    FieldInfo* Script_env = il2cpp_class_get_field_from_name(script_class, "env");
    FieldInfo* LuaBase_Interpreter = il2cpp_class_get_field_from_name(luabase_class, "cdn"); // _Interpreter
    FieldInfo* LuaBase_Reference = il2cpp_class_get_field_from_name(luabase_class, "cdm"); // _Reference
    FieldInfo* Lua_luaState = il2cpp_class_get_field_from_name(lua_class, "luaState");
    FieldInfo* InstanceManager_DebugWindow = il2cpp_class_get_field_from_name(instance_manager_class, "DebugWindow");
    FieldInfo* DebugWindow_NavigationButton = il2cpp_class_get_field_from_name(debugwindow_class, "NavigationButton");
    const PropertyInfo* Component_gameObject = il2cpp_class_get_property_from_name(component_class, "gameObject");

    if (Dynamic_Name == nullptr || Script_env == nullptr || Lua_luaState == nullptr || LuaBase_Interpreter == nullptr || LuaBase_Reference == nullptr 
        || InstanceManager_DebugWindow == nullptr || DebugWindow_NavigationButton == nullptr || Component_gameObject == nullptr)
        return false;
    std::printf("Loaded properties!\n");

    dynamic_name_off = il2cpp_field_get_offset(Dynamic_Name);
    script_env_off = il2cpp_field_get_offset(Script_env);
    luabase_interpreter_off = il2cpp_field_get_offset(LuaBase_Interpreter);
    luabase_reference_off = il2cpp_field_get_offset(LuaBase_Reference);
    lua_luastate_off = il2cpp_field_get_offset(Lua_luaState);
    instancemanager_debugwindow_off = il2cpp_field_get_offset(InstanceManager_DebugWindow);
    debugwindow_navigationbutton_off = il2cpp_field_get_offset(DebugWindow_NavigationButton);

    const MethodInfo* Component_get_gameObject = il2cpp_property_get_get_method(const_cast<PropertyInfo*>(Component_gameObject));
    const MethodInfo* String_CreateString = il2cpp_class_get_method_from_name(string_class, "CreateString", 3);
    const MethodInfo* LocalScript_ctor = il2cpp_class_get_method_from_name(localscript_class, ".ctor", 0);
    const MethodInfo* LocalScript_RegisterTable = il2cpp_class_get_method_from_name(localscript_class, "RegisterTable", 0);
    const MethodInfo* Script_LoadScript = il2cpp_class_get_method_from_name(script_class, "LoadScript", 1);
    const MethodInfo* ScriptManager_Update = il2cpp_class_get_method_from_name(scriptmanager_class, "Update", 0);
    const MethodInfo* ScriptManager_Log = il2cpp_class_get_method_from_name(scriptmanager_class, "Log", 1);
    const MethodInfo* GameObject_SetActive = il2cpp_class_get_method_from_name(gameobject_class, "SetActive", 1);

    if (LocalScript_ctor == nullptr || Script_LoadScript == nullptr || LocalScript_RegisterTable == nullptr || ScriptManager_Update == nullptr || Component_get_gameObject == nullptr
        || GameObject_SetActive == nullptr || ScriptManager_Log == nullptr)
        return false;
    std::printf("Loaded methods!\n");

    string_createstring = reinterpret_cast<string_createstring_t>(String_CreateString->methodPointer);
    localscript_ctor = reinterpret_cast<localscript_ctor_t>(LocalScript_ctor->methodPointer);
    script_loadscript = reinterpret_cast<script_loadscript_t>(Script_LoadScript->methodPointer);
    localscript_registertable = reinterpret_cast<localscript_registertable_t>(LocalScript_RegisterTable->methodPointer);
    scriptmanager_update = reinterpret_cast<scriptmanager_update_t>(ScriptManager_Update->methodPointer);
    scriptmanager_log = reinterpret_cast<scriptmanager_log_t>(ScriptManager_Log->methodPointer);
    get_gameobject = reinterpret_cast<get_gameobject_t>(Component_get_gameObject->methodPointer);
    gameobject_setactive = reinterpret_cast<gameobject_setactive_t>(GameObject_SetActive->methodPointer);

    if (!set_singleton<Il2CppObject*>(instance_manager_class, "singleton", instance_manager))
        return false;

    il2cpp_thread_detach(current_thread);
    return true;
}

void cheat()
{
    make_console();
    std::printf("Running AiDz professional edition.\n");

    if (update_chair() && communication.setup())
    {
        std::thread([]() -> void {
            communication.receive_data();
        }).detach();
        std::printf("AiDz successfully injected!\n");
    }
    else
    {
        std::printf("There was an issue with AiDz! Failed to setup.\n");
        return;
    }

    // Wait until game is ready for execution
    bool game_is_ready = false;
    while (!set_singleton<bool>(script_class, "ddh", game_is_ready)) // StaticEnvSetup
        continue;

    set_debug_visible(true);
    setup_hooks();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        std::thread(cheat).detach();
        break;
    }
    return TRUE;
}

// next: add anti-afk since game kicks for afk timeouts (already found part of the mechanism)


// ideas to add to lua engine:
// script will make its own closure and set it in the script field, not the game ( more control ), too much to implement instead make a dummy script and swap data*
// debug functions unlock
// getscriptclosure(script: Script)
// setscriptclosure(script: Script)
// setfenv
// decompile(script: Script)
// unrestricted NLua access ( bypass their index hook )

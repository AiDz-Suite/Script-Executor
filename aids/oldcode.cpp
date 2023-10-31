std::uint64_t __fastcall scriptmanager_update_hook(std::uintptr_t script_manager)
{
    std::uintptr_t nstate = 0;
    lua_State* lua_state = nullptr;

    if (std::uintptr_t _a = *reinterpret_cast<std::uintptr_t*>(il2cpp_base + 0x0F17190))
    {
        if (std::uintptr_t _a1 = *reinterpret_cast<std::uintptr_t*>(_a + 0xB8))
        {
            nstate = *reinterpret_cast<std::uintptr_t*>(_a1);
            if (nstate)
            {
                lua_state = *reinterpret_cast<lua_State**>(nstate + 0x38);
            }
        }
    }

    if (!nstate || !lua_state)
        return original_scriptmanager_update(script_manager);


    // Set currently executing to prevent internal issues.
    *reinterpret_cast<std::uint8_t*>(nstate + 0x61) = 1;

    static bool ran = false;

    if (!ran && lua_state)
    {
        ran = true;
        debug_unpatch();

        int old_top = lua_gettop(lua_state);
        std::uintptr_t local_script = run_lua_script(R"---(
            print("Debug window successfully unlocked!")
            printWarning("AiDZ loaded successfully!")
            print("Welcome to AiDz " .. Players.LocalPlayer.Username)

            printWarning("Retrieving server list...")
            for i,v in pairs(Players.GetPlayers()) do 
                print("Found Player: " .. v.Username)
            end

            Players.PlayerJoined:connect(function(player)
                printWarning("Player: " .. player.Username .. " has joined!")
            end)

            function big_hack()
                Players.LocalPlayer.WalkSpeed = 10
                Players.LocalPlayer.Size = Vector3.new(3, 5, 3)
            end
        )---");
        lua_settop(lua_state, old_top);
    }

    // No longer executing
    *reinterpret_cast<std::uint8_t*>(nstate + 0x61) = 0;
    return original_scriptmanager_update(script_manager);
}

enum class admin_commands : std::uint8_t
{
    speed,
    jump,
    big,
    smaller,
    print,
    debugon,
    debugoff
};

std::unordered_map<std::wstring, admin_commands> commands =
{
    {L"speed", admin_commands::speed},
    {L"jump", admin_commands::jump},
    {L"big", admin_commands::big},
    {L"small", admin_commands::smaller},
    {L"print", admin_commands::print},
    {L"debugon", admin_commands::debugon},
    {L"debugoff", admin_commands::debugoff},
};

bool __fastcall networkclient_send_hook(std::uintptr_t network_client, std::uint16_t msg_type, std::uintptr_t msg)
{
    switch (msg_type)
    {
    case 0x61B6: // Brickplanet_MsgChatSend
    {
        std::wstring spec_message{ reinterpret_cast<wchar_t*>(*reinterpret_cast<std::uintptr_t*>(msg + 0x10) + 0x14) };

        if (auto command = commands.find(spec_message); command != commands.end())
        {
            switch (command->second)
            {
            case admin_commands::speed:
                run_lua_script("Players.LocalPlayer.WalkSpeed = 10");
                break;
            case admin_commands::jump:
                run_lua_script("Players.LocalPlayer.JumpPower = 10");
                break;
            case admin_commands::big:
                run_lua_script("Players.LocalPlayer.Size = Vector3.new(3, 5, 3)");
                break;
            case admin_commands::smaller:
                run_lua_script("Players.LocalPlayer.Size = Vector3.new(0.25, 0.25, 0.25)");
                break;
            case admin_commands::print:
                run_lua_script("print('Hey I ran!')");
                break;
            case admin_commands::debugon:
                debug_any_game(true);
                break;
            case admin_commands::debugoff:
                debug_any_game(false);
                break;
            default:
            {
                std::printf("Invalid command!\n");
                break;
            }
            }

            // No sending commands to real chat.
            return true;
        }

        break;
    }
    default:
    {
        // std::printf("[NET HOOK]: Received unknown packet: 0x%04X\n", msg_type);
        break;
    }
    }

    return original_networkclient_send(network_client, msg_type, msg);
}

std::uintptr_t __fastcall unitywebrequest_sendwebrequest_hook(std::uintptr_t unitywebrequest)
{
    std::uintptr_t url = unitywebrequest_geturl(unitywebrequest);
    if (url)
    {
        std::wstring http_url{ reinterpret_cast<wchar_t*>(url + 0x14) };
        std::wcout << L"Outbound request being sent to: " << http_url << std::endl;
    }

    return original_unitywebrequest_sendwebrequest(unitywebrequest);
}

int test_function(lua_State* state)
{
    std::printf("custom function called!\n");

    for (int i = 1; i <= lua_gettop(state); ++i)
    {
        switch (lua_type(state, i))
        {
        case LUA_TNIL:
            std::printf("nil\n");
            break;
        case LUA_TNUMBER:
            std::printf("%.2f\n", lua_tonumber(state, i));
            break;
        case LUA_TSTRING:
            std::printf("%s\n", lua_tostring(state, i));
            break;
        case LUA_TBOOLEAN:
            std::printf("%s\n", lua_toboolean(state, i) ? "true" : "false");
            break;
        default:
            std::printf("unk: %s\n", lua_typename(state, lua_type(state, i)));
            break;
        }
    }

    return 0;
}

// Optionally add my own shit into the script
void configure_script_env(lua_State* script_thread)
{
    lua_pushcfunction(script_thread, test_function);
    lua_setglobal(script_thread, "cheatfunction");
}

void debug_unpatch()
{
    // Unpatch prevention of outputing to debug window (it checks ownerid before even doing it, no ownerid check, no problem)
    DWORD old = 0;
    VirtualProtect(reinterpret_cast<LPVOID>(debug_check_patch), 2, PAGE_EXECUTE_READWRITE, &old);
    *reinterpret_cast<std::uint16_t*>(debug_check_patch) = 0x9090;
    VirtualProtect(reinterpret_cast<LPVOID>(debug_check_patch), 2, old, &old);
}

void debug_any_game(bool enable)
{
    std::uintptr_t instance_manager = **reinterpret_cast<std::uintptr_t**>(*reinterpret_cast<std::uintptr_t*>(il2cpp_base + 0x0F16FD0) + 0xB8);
    if (!instance_manager)
        return;

    std::uintptr_t debug_window = *reinterpret_cast<std::uintptr_t*>(instance_manager + 0x48);
    if (!debug_window)
        return;

    // Allow debug window to be shown
    std::uintptr_t NavButton = *reinterpret_cast<std::uintptr_t*>(debug_window + 0x18);
    if (!NavButton)
        return;

    std::uintptr_t game_obj = get_gameobject(NavButton);
    if (!game_obj)
        return;

    gameobject_setactive(game_obj, enable);
}

// Old code

using get_gameobject_t = std::uintptr_t(__fastcall*)(std::uintptr_t unity_object);
get_gameobject_t get_gameobject = *reinterpret_cast<get_gameobject_t*>(il2cpp_base + 0x0F93F40);

using gameobject_setactive_t = void(__fastcall*)(std::uintptr_t game_object, bool visible);
gameobject_setactive_t gameobject_setactive = reinterpret_cast<gameobject_setactive_t>(il2cpp_base + 0x991580);

using networkclient_send_t = bool(__fastcall*)(std::uintptr_t network_client, std::uint16_t msg_type, std::uintptr_t msg);
networkclient_send_t hooked_networkclient_send = reinterpret_cast<networkclient_send_t>(il2cpp_base + 0x941710);
networkclient_send_t original_networkclient_send;

using unitywebrequest_sendwebrequest_t = std::uintptr_t(__fastcall*)(std::uintptr_t unitywebrequest);
unitywebrequest_sendwebrequest_t hooked_unitywebrequest_sendwebrequest = reinterpret_cast<unitywebrequest_sendwebrequest_t>(il2cpp_base + 0x5F9BD0);
unitywebrequest_sendwebrequest_t original_unitywebrequest_sendwebrequest;

using unitywebrequest_geturl_t = std::uintptr_t(__fastcall*)(std::uintptr_t unitywebrequest);
unitywebrequest_geturl_t unitywebrequest_geturl = reinterpret_cast<unitywebrequest_geturl_t>(il2cpp_base + 0x5FA740);


std::uintptr_t debug_check_patch = il2cpp_base + 0x6497FD;



/*
    Script:
    0xC8 = Script Running
    0x18 = Dynamic Proxies
    0xD0 = environment
    0xD8 = update func
    0xE0 = message
    0xA0 = script name
    0x10 = script time
    0x0 = static env
    0x8 = env setup

    Internal_CoroutineCreator - function
    setmetatable - function
    rawlen - function
    luanet - table
    type - function
    OnUpdate - function
    dofile - function
    math - table
    load - function
    getmetatable - function
    pcall - function
    pairs - function
    rawset - function
    import - function
    -8026 - table
    debug - table
    coroutine - table
    InternalTime - number
    next - function
    CLRPackage - function
    printApi - function
    print - userdata
    Internal_DynamicsTable - table
    os - table
    collectgarbage - function
    InternalTempTable - table
    Api_Error - function
    string - table
    rawget - function
    Internal_CoroutineRemover - function
    sandbox_env - table
    wait - function
    Referenced_Coroutines - table
    tostring - function
    tonumber - function
    require - function
    io - table
    printtable - function
    table - table
    Internal_CoroutineRunner - function
    copyImports - function
    count - number
    bit32 - table
    xpcall - function
    _G - table
    loadfile - function
    Internal_CoroutineCreatorAction - function
    rawequal - function
    ipairs - function
    package - table
    select - function
    _VERSION - string
    error - function
    assert - function

    Sandbox:
    wait - function
    tonumber - function
    math - table
    tostring - function
    Referenced_Coroutines - table
    string - table
    ipairs - function
    pairs - function
    pcall - function
    type - function
    Internal_CoroutineRemover - function
    table - table
    next - function
    os - table
    Internal_CoroutineRunner - function
    OnUpdate - function
    coroutine - table
    Internal_CoroutineCreator - function



    print table:
    lua_pushglobaltable(script_thread);
    lua_pushnil(script_thread);

    while (lua_next(script_thread, -2) != 0)
    {
        if (lua_isstring(script_thread, -2))
        {
            const char* global_name = lua_tostring(script_thread, -2);
            std::printf("[INTERNAL]: %s - %s\n", global_name, lua_typename(script_thread, lua_type(script_thread, -1)));
        }
        lua_pop(script_thread, 1);
    }
*/


// Net hook for spying on packets / chat hook
MH_CreateHook(hooked_networkclient_send, &networkclient_send_hook, reinterpret_cast<LPVOID*>(&original_networkclient_send));
MH_EnableHook(hooked_networkclient_send);

// HTTP hook to view traffic
MH_CreateHook(hooked_unitywebrequest_sendwebrequest, &unitywebrequest_sendwebrequest_hook, reinterpret_cast<LPVOID*>(&original_unitywebrequest_sendwebrequest));
MH_EnableHook(hooked_unitywebrequest_sendwebrequest);
#include "lua-script.hpp"
#include <iostream>
#include <cstring>

// A sample C++ function that the Lua script will call.
// The function prints its argument and returns the length of the string.
static int cpp_print(lua_State* L) {
    // Check that there is at least one argument and that it is a string.
    if (!lua_isstring(L, 1)) {
        lua_pushstring(L, "Incorrect argument to 'cpp_print'. Expected a string.");
        lua_error(L);
        return 0;  // never reached
    }

    // Get the string from the first argument.
    const char* message = lua_tostring(L, 1);
    std::cout << "C++ received from Lua: " << message << std::endl;

    // Push the result (length of the string) onto the Lua stack.
    lua_pushnumber(L, static_cast<lua_Number>(strlen(message)));
    // Return 1 value.
    return 1;
}

int lua_example(){
    // Create a new Lua state.
    lua_State *L = luaL_newstate();
    if (L == nullptr) {
        std::cerr << "Failed to create Lua state." << std::endl;
        return 1;
    }

    // Open the standard Lua libraries.
    luaL_openlibs(L);

    // Attach the C++ function to Lua.
    // We push the function and give it a name in the global table.
    lua_pushcfunction(L, cpp_print);
    lua_setglobal(L, "cpp_print");

    // A simple Lua script in a string.
    const char* lua_script = R"(
        print("Lua says hello!")
        local message = "Hello from Lua!"
        -- Call the C++ function and receive the result.
        local len = cpp_print(message)
        print("Lua received length:", len)
    )";

    // Run the Lua script.
    if (luaL_dostring(L, lua_script) != LUA_OK) {
        // If there is an error, get the error message from the stack.
        const char *error = lua_tostring(L, -1);
        std::cerr << "Error running Lua script: " << error << std::endl;
        lua_pop(L, 1);  // remove error message from the stack
    }

    // Close the Lua state.
    lua_close(L);
    return 0;
}


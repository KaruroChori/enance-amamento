#pragma once
#include <lua.hpp>

/*
Ok, real talk. The reason for lua to be here is to provide a easy scripting option for events and animations which must affect our UI visualization. That's it really. 
To do that, we really need only a small number of functions provided from the outside.
- an absolute time unit and the expected deltaTime of this frame.
- getter for tree attributes
- setter for tree atributes
- camera info (not so important)
- raycast info (not so important)
- mouse info (not so important)
That is it. As long as we can provide such interface, all is good.
*/

int lua_example();
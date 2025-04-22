#pragma once

/**
 * @file optimized.hpp
 * @author karurochari
 * @brief An SDF representation switcher. 
 * @details It dynamically selects between OctaTree/Interpreted/Dynlib based on the fact they have been calculated, and if the current state is dirty waiting for recomputiation.
 * @date 2025-03-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#ifndef SDF_INTERNALS
#error "Don't import manually, this can only be used internally by the library"
#endif

#include "../sdf.hpp"
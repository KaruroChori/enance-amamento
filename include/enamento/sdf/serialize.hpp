#pragma once 

/**
 * @file serialize.hpp
 * @author karurochari
 * @brief Serialization for sdf.
 * @date 2025-04-19
 * It covers serialization & deserialization of attributes, and several helpers to implement code generation.
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef SDF_INTERNALS
#error Don't import manually, this can only be used internally by the library
#endif

#include "sdf.hpp"
#include <ostream>
#include <format>

namespace sdf{

namespace serialize{


bool fields2cpp (std::ostream& out, const void * node, fields_t fields, bool trailing_comma=true);
bool map2fields (const std::map<std::string,std::string>& map, const void * node, fields_t fields);




}

}
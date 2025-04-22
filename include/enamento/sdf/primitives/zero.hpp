#pragma once

/**
 * @file zero.hpp
 * @author karurochari
 * @brief Basic SDF for nothing. Used as placeholder
 * @date 2025-03-23
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#ifndef SDF_INTERNALS
#error "Don't import manually, this can only be used internally by the library"
#endif

#include <cmath>
#include <cstddef>
#include <sstream>

#include "../sdf.hpp"
#include "../tree.hpp"


namespace sdf{

    namespace configs{
    }

    namespace{namespace impl_base{

        template <typename Attrs=default_attrs>
        struct Zero{
            using attrs_t = Attrs;
            [[no_unique_address]] Attrs::extras_t cfg;

            constexpr Zero(Attrs::extras_t cfg={}):cfg(cfg){}

            constexpr inline float sample(const glm::vec3&)const {return INFINITY;}

            constexpr inline void traits(traits_t& to) const{
                PRIMITIVE_TRAIT_SYM;
                PRIMITIVE_TRAIT_GOOD;
                to.outer_box={glm::vec3(-INFINITY),glm::vec3(INFINITY)};
            }

            constexpr inline static const char* _name = "Zero";

            constexpr inline static field_t _fields[] = {};

            PRIMITIVE_COMMONS
        };
        
    }}

    sdf_register_primitive(Zero);
}


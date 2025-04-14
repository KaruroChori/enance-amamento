#pragma once

/**
 * @file box.hpp
 * @author karurochari
 * @brief Basic SDF for a box, centered at the origin. The 2D version is for a rect.
 * @date 2025-03-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */


#ifndef SDF_INTERNALS
#error Don't import manually, this can only be used internally by the library
#endif

#include <cmath>
#include "../sdf.hpp"

namespace sdf{

    namespace configs{
    }

    namespace{namespace impl_base{

        template <typename Attrs=default_attrs>
        struct Box{
            using attrs_t = Attrs;
            [[no_unique_address]] Attrs::extras_t cfg;
            vec3 b;

            constexpr Box(const vec3& b, Attrs::extras_t cfg={}):cfg(cfg),b(b){}
            constexpr Box(Attrs::extras_t cfg={}):cfg(cfg){}

            constexpr inline float sample(const glm::vec3& pos)const {
                vec3 q = abs(pos) - b;
                return length(max(q,0.0f)) + min(max(q.x,max(q.y,q.z)),0.0f);
            }

            constexpr inline void traits(traits_t& to) const{
                PRIMITIVE_TRAIT_SYM;
                PRIMITIVE_TRAIT_GOOD;
                to.outer_box={-b,b};
            }

            constexpr inline static const char* _name = "Box";

            constexpr inline static field_t _fields[] = {
                FIELD(Box,vec3,deftype,b, "Size in the three axis",vec3(0,0,0),vec3(INFINITY,INFINITY,INFINITY),vec3(1,1,1),nullptr),
            };

            PRIMITIVE_COMMONS
        };
    }}

    sdf_register_primitive(Box);
}
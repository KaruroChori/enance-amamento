#pragma once

/**
 * @file join.hpp
 * @author karurochari
 * @brief Join, or Union. Boolean operation with harsh edges.
 * @date 2025-03-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef SDF_INTERNALS
#error Don't import manually, this can only be used internally by the library
#endif

#include "../../sdf.hpp"

namespace sdf{

    namespace configs{
        struct Join{};
    }

    namespace{namespace impl_base{

        template <typename L, typename R>
        struct Join : utils::binary_op<L, R>{
            using base = utils::binary_op<L, R>;
            using base::base;

            constexpr float sample(const glm::vec3& pos) const{
                auto& left = base::left();
                auto& right = base::right();
                auto lres = left.sample(pos);
                auto rres = right.sample(pos);
                return min(lres,rres);
            }

            constexpr base::attrs_t operator()(const glm::vec3& pos) const{
                auto& left = base::left();
                auto& right = base::right();
                auto lres = left(pos);
                auto rres = right(pos);
                float distance = min(lres.distance,rres.distance);
                return {distance,normals(pos),(distance<MIX_EPS)?(lres+rres): typename base::attrs_t::extras_t{}};
            }

            constexpr inline void traits(const traits_t& fromA, const traits_t& fromB, traits_t& to) const{
                to.is_sym[0]=(fromA.is_sym[0]==true && fromB.is_sym[0] == true)?true:tribool::unknown;
                to.is_sym[1]=(fromA.is_sym[1]==true && fromB.is_sym[1] == true)?true:tribool::unknown;
                to.is_sym[2]=(fromA.is_sym[2]==true && fromB.is_sym[2] == true)?true:tribool::unknown;

                to.is_exact_inner=tribool::unknown;
                to.is_exact_outer=(fromA.is_exact_outer==true && fromB.is_exact_outer==true)?true:tribool::unknown;
                to.is_bounded_inner=(fromA.is_bounded_inner==true && fromB.is_bounded_inner==true)?true:tribool::unknown;
                to.is_bounded_outer=(fromA.is_bounded_outer==true && fromB.is_bounded_outer==true)?true:tribool::unknown;
                to.outer_box={ min(fromA.outer_box.min,fromB.outer_box.min), max(fromA.outer_box.max,fromB.outer_box.max)  };
            }

            constexpr inline void traits(traits_t& to) const{
                traits_t ltraits, rtraits;
                (base::left()).traits(ltraits);(base::right()).traits(rtraits);
                traits(ltraits,rtraits,to);
            }

            constexpr inline static const char* _name = "Join";

            constexpr inline static field_t _fields[] = {};
            PRIMITIVE_NORMAL
        };

    }}

    sdf_register_operator_2o(Join, +);
}
#pragma once

/**
 * @file material.hpp
 * @author karurochari
 * @brief Changes material for child.
 * @date 2025-03-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef SDF_INTERNALS
#error "Don't import manually, this can only be used internally by the library"
#endif

#include "../sdf.hpp"

namespace sdf{

    namespace configs{
        template <typename L>
        struct Material{
            typename L::attrs_t material;   //TODO: I need to change this one to support the shared ptr version.
        };
    }

    namespace{namespace impl_base{

        template <typename L>
        struct Material : utils::unary_op<L, configs::Material<L>>{
            using base = utils::unary_op<L, configs::Material<L>>;
            using base::base;

            constexpr inline float sample(const glm::vec3& pos) const{return base::left().sample(pos);}

            constexpr inline base::attrs_t operator()(const glm::vec3& pos) const{
                auto& left = base::left();
                auto lres = left.sample(pos);
                return {lres,normals(pos),this->cfg};
            }

            constexpr inline void traits(const traits_t& from, const traits_t&, traits_t& to) const{
                to.is_sym=from.is_sym;
                to.is_exact_inner=from.is_exact_inner;
                to.is_exact_outer=from.is_exact_outer;
                to.is_bounded_inner=from.is_bounded_inner;
                to.is_bounded_outer=from.is_bounded_outer;
                to.outer_box={from.outer_box.min*this->cfg.offset,from.outer_box.max*this->cfg.offset};
            }

            constexpr inline void traits(traits_t& to) const{
                traits_t ltraits;
                (base::left()).traits(ltraits);
                traits(ltraits,ltraits,to);
            }

            constexpr inline static const char* _name = "Material";

            constexpr inline static field_t _fields[] = {
                {false,field_t::type_cfg, field_t::widget_deftype, "material", "Material override", offsetof(Material, cfg)+offsetof(typename base::cfg_t, material) , sizeof(base::cfg.material), nullptr, nullptr, SVal<typename L::attrs_t{}>, nullptr}
            };

            PRIMITIVE_NORMAL
        };
    }}

    sdf_register_operator_1(Material);  
}
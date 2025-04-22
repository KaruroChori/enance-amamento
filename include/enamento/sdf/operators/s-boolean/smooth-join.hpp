#pragma once
#include "../../sdf.hpp"
//TODO To write.

#ifndef SDF_INTERNALS
#error "Don't import manually, this can only be used internally by the library"
#endif


namespace sdf{

    namespace configs{
        struct SmoothJoin{float factor;};
    }

    namespace{namespace impl_base{

        template <typename L, typename R>
        struct SmoothJoin : utils::binary_op<L, R, configs::SmoothJoin>{
            using base = utils::binary_op<L, R, configs::SmoothJoin>;
            using base::base;

            constexpr float sample(const glm::vec3& pos) const{
                auto& left = base::left();
                auto& right = base::right();

                auto lres = left.sample(pos);
                auto rres = right.sample(pos);

                float h = clamp( 0.5 + 0.5*(rres-lres)/this->cfg.factor, 0.0, 1.0 );
                return mix( rres, lres, h ) - this->cfg.factor*h*(1.0-h);
            }

            constexpr base::attrs_t operator()(const glm::vec3& pos) const{
                auto& left = base::left();
                auto& right = base::right();

                auto lres = left(pos);
                auto rres = right(pos);

                float h = clamp( 0.5 + 0.5*(rres.distance-lres.distance)/this->cfg.factor, 0.0, 1.0 );
                float distance = mix( rres.distance, lres.distance, h ) - this->cfg.factor*h*(1.0-h);
                return {distance,normals(pos),(distance<EPS)?(lres+rres): typename base::attrs_t::extras_t{}};
            }

            constexpr inline void traits(const traits_t& fromA, const traits_t& fromB, traits_t& to) const{
                //TODO:
            }

            constexpr inline void traits(traits_t& to) const{
                traits_t ltraits, rtraits;
                (base::left()).traits(ltraits);(base::right()).traits(rtraits);
                traits(ltraits,rtraits,to);
            }

            constexpr inline static const char* _name = "SmoothJoin";

            constexpr inline static field_t _fields[] = {
                FIELD_OP_R(SmoothJoin,float,deftype,factor, "Radius of smoothing")
            };
            
            PRIMITIVE_NORMAL
        };

    }}

    sdf_register_operator_2(SmoothJoin);
}
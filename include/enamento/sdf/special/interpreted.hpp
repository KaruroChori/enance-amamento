#pragma once
/**
 * @file dynlib-sdf.hpp
 * @author karurochar
 * @brief Special wrapper loading symbols of an SDF compiled as dynamic library, and exposing it as any other
 * @date 2025-03-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <dlfcn.h>



#ifndef SDF_INTERNALS
#error Don't import manually, this can only be used internally by the library
#endif

#include <cmath>
#include "../sdf.hpp"

namespace sdf{

    namespace configs{
    }

    namespace{namespace impl{
        template<typename Attrs=default_attrs>
        struct Interpreted{
            private:
                //typedef handle_t utils::tree_idx<Attrs> *;
                //handle_t _handle = nullptr;

                typedef size_t handle_t;
                handle_t _handle = 0;

                inline utils::tree_idx<Attrs> * handle() const{
                    //TODO #if to handle it as direct pointer?
                    uint32_t offset;
                    memcpy(&offset,global_shared[_handle].base,4);
                    return (utils::tree_idx<Attrs> *)((uint8_t*)global_shared[_handle].base+offset);
                }

            public:
            using attrs_t = Attrs;

            Interpreted(handle_t h):_handle(h){}
            
            inline Attrs operator()(const glm::vec3& pos) const{return handle()->operator()(pos);};
            inline float sample(const glm::vec3& pos) const{return handle()->sample(pos);}
            inline void traits(traits_t& out) const{return handle()->traits(out);}
            
            inline const char* name() const{return handle()->name();}
            inline fields_t fields() const{return handle()->fields();}
            inline fields_t fields(const path_t* steps) const{return handle()->fields(steps);};
            inline visibility_t is_visible() const{return visibility_t::VISIBLE;}

            inline bool to_cpp(ostream& out)const{return handle()->to_cpp(out);};
            inline bool to_xml(xml& out)const{return handle()->to_xml(out);}
            inline uint64_t to_tree(tree::builder& out)const{return handle()->to_tree(out);}
            inline bool from_xml(const xml& in){return handle()->from_xml(in);}

        };
    }}

    namespace comptime {
        template <typename Attrs=default_attrs>
        using Interpreted_t = utils::primitive<Attrs,impl::Interpreted>::type;
        template <typename Attrs=default_attrs>
        constexpr inline Interpreted_t<Attrs> Interpreted (impl::Interpreted<Attrs> && ref ){
            return ref;
        }
    }                   
    namespace polymorphic {
        template <typename Attrs=default_attrs>
        using Interpreted_t = utils::dyn<Attrs,impl::Interpreted>;
        template <typename Attrs=default_attrs>
        constexpr inline Interpreted_t<Attrs> Interpreted (impl::Interpreted<Attrs> && ref ){
            return ref;
        }
    }
    namespace dynamic {
        template <typename Attrs=default_attrs>
        using Interpreted_t =utils::dyn<Attrs,impl::Interpreted>;
        template <typename Attrs=default_attrs>
        constexpr inline std::shared_ptr<utils::base_dyn<Attrs>> Interpreted (impl::Interpreted<Attrs> && ref ){
            std::shared_ptr<utils::base_dyn<Attrs>> tmp = std::make_shared<utils::dyn<Attrs,impl::Interpreted>>(utils::dyn<Attrs,impl::Interpreted>(ref));
            return tmp; 
        }               
    }                   
    
}


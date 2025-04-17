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
        struct Dynlib{
            private:
                void *dl_handle = nullptr;
        
                Attrs (*_operator)(const glm::vec3& pos) = nullptr;
                float (*_sample)(const glm::vec3& pos) = nullptr;

                void (*_traits)(traits_t& out) = nullptr;
                const char* (*_name)() = nullptr;
                fields_t (*_fields)() = nullptr;

                bool (*_to_cpp)(ostream& out) = nullptr;
                bool (*_to_xml)(xml& out) = nullptr;
                bool (*_from_xml)(const xml& in) = nullptr;

            public:
        
            Dynlib(void *dl_handle, int device):dl_handle(dl_handle){
                _operator = (decltype(_operator))((void*(*)(int))dlsym(dl_handle, "addr__operator"))(device);
                _sample = (decltype(_sample))((void*(*)(int))dlsym(dl_handle, "addr__sample"))(device);
                _traits = (decltype(_traits))((void*(*)(int))dlsym(dl_handle, "addr__traits"))(device);
                _name = (decltype(_name))((void*(*)(int))dlsym(dl_handle, "addr__name"))(device);
                _fields = (decltype(_fields))((void*(*)(int))dlsym(dl_handle, "addr__fields"))(device);
                _to_cpp = (decltype(_to_cpp))((void*(*)(int))dlsym(dl_handle, "addr__to_cpp"))(device);
                _to_xml = (decltype(_to_xml))((void*(*)(int))dlsym(dl_handle, "addr__to_xml"))(device);
                _from_xml = (decltype(_from_xml))((void*(*)(int))dlsym(dl_handle, "addr__from_xml"))(device);
                //TODO: Add to_tree
                //TODO: Add get
            }
        
            inline Attrs operator()(const glm::vec3& pos) const{return _operator(pos);};
            inline float sample(const glm::vec3& pos) const{return _sample(pos);}
            inline void traits(traits_t& out) const{return _traits(out);}
            inline const char* name() const{return _name();}
            inline fields_t fields() const{return _fields();}
            inline bool to_cpp(ostream& out)const{return _to_cpp(out);};
            inline bool to_xml(xml& out)const{return _to_xml(out);}
            inline bool from_xml(const xml& in){return _from_xml(in);}
        };
    }}

    namespace comptime {                                                                                            \
        template <typename Attrs=default_attrs>                                                                     \
        using Dynlib_t = utils::primitive<Attrs,impl::Dynlib >::type;                                               \
        template <typename Attrs=default_attrs>                                                                     \
        constexpr inline Dynlib_t<Attrs> Dynlib (  impl::Dynlib<Attrs> && ref ){                                    \
            return ref;                                                                                             \
        }                                                                                                           \
    }                                                                                                               \
    namespace polymorphic {                                                                                         \
        template <typename Attrs=default_attrs>                                                                     \
        using Dynlib_t = utils::dyn<Attrs,impl::Dynlib >;                                                           \
        template <typename Attrs=default_attrs>                                                                     \
        constexpr inline Dynlib_t<Attrs> Dynlib (  impl::Dynlib<Attrs> && ref ){                                    \
            return ref;                                                                                             \
        }                                                                                                           \
    }                                                                                                               \
    namespace dynamic {                                                                                             \
        template <typename Attrs=default_attrs>                                                                     \
        using Dynlib_t =utils::dyn<Attrs,impl::Dynlib >;                                                            \
        template <typename Attrs=default_attrs>                                                                     \
        constexpr inline std::shared_ptr<utils::base_dyn<Attrs>> Dynlib (  impl::Dynlib<Attrs> && ref ){            \
            std::shared_ptr<utils::base_dyn<Attrs>> tmp =                                                           \
                std::make_shared<utils::dyn<Attrs,impl::Dynlib>>(utils::dyn<Attrs,impl::Dynlib>(ref));              \
            return tmp;                                                                                             \
        }                                                                                                           \
    }                                                                                                               \
    
}


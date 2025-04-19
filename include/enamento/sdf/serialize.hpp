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

namespace codegen{

#define HFIELD(TYPE) (*(TYPE*)(((uint8_t*)&node)+field.offset))
#define HFIELD_DOT(TYPE, SUB) (*(TYPE*)(((uint8_t*)&node)+field.offset)).SUB

//This works with operators too thanks to the magic trick that cfg is the first argument for them. So this and cfg are pointing to the same memcell
template<typename T>
bool to_cpp_from_fields(sdf::ostream& out, const T& node, bool trailing_comma=true){  //TODO: add constraint
    #if SDF_IS_HOST==true
    bool first=true;
    for( sdf::field_t field : node.fields()){
        if(!first)out<<",";
        else first=false;
        switch(field.type){
            case field_t::type_unknown:
                return false;
            case field_t::type_cfg:
                return false;
            case field_t::type_float:
                out<<std::format("{:f}f",HFIELD(float));
                break;
            case field_t::type_int:
                out<<std::format("{}",HFIELD(int));
                break;
            case field_t::type_vec2:
                out<<std::format("{{{:f}f,{:f}f}}",HFIELD_DOT(glm::vec2,x),HFIELD_DOT(glm::vec2,y));
                break;
            case field_t::type_vec3:
                out<<std::format("{{{:f}f,{:f}f,{:f}f}}",HFIELD_DOT(glm::vec3,x),HFIELD_DOT(glm::vec3,y),HFIELD_DOT(glm::vec3,z));
                break;
            case field_t::type_ivec2:
                out<<std::format("{{{},{}}}",HFIELD_DOT(glm::ivec2,x),HFIELD_DOT(glm::ivec2,y));
                break;
            case field_t::type_ivec3:
                out<<std::format("{{{},{},{}}}",HFIELD_DOT(glm::ivec3,x),HFIELD_DOT(glm::ivec3,y),HFIELD_DOT(glm::ivec3,z));
                break;
            case field_t::type_bool:
                out<<HFIELD(bool);
                break;
            case field_t::type_tribool:
                return false;   //TODO
            case field_t::type_enum:
                out<<HFIELD(int);
                break;
            case field_t::type_shared_buffer:
                out<<HFIELD(size_t);
                break;
            default:
                break;
        }
    }
    if(trailing_comma && node.fields().items!=0)out<<",";

    return true;
    #else
    return false;
    #endif
}

template<typename T>
bool to_xml_from_fields(sdf::xml& root, const T& node){
    #if SDF_IS_HOST==true
    for( sdf::field_t field : node.fields()){
        switch(field.type){
            //TODO: implement
        }
    }
    return true;
    #else
    return false;
    #endif
}

#undef HFIELD
#undef HFIELD_DOT



}

}
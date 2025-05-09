#define SDF_HEADLESS
#include "sdf/commons.hpp"
#include <format>

namespace sdf{
namespace serialize{

#define HFIELD(TYPE) (*(TYPE*)(((uint8_t*)node)+field.offset))
#define HFIELD_DOT(TYPE, SUB) (*(TYPE*)(((uint8_t*)node)+field.offset)).SUB

bool fields2cpp (std::ostream& out, const void * node, fields_t fields, bool trailing_comma){
    bool first=true;
    for( sdf::field_t field : fields){
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
    if(trailing_comma && fields.items!=0)out<<",";

    return true;
}

bool map2fields (const std::map<std::string,std::string>& map, const void * node, fields_t fields){

}

#undef HFIELD
#undef HFIELD_DOT

}
}
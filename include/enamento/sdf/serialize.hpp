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

#include "sdf/commons.hpp"

#include <ostream>
#include <pugixml.hpp>

namespace sdf{

namespace serialize{

/**
 * @brief Convert fields from an SDF node to cpp code.
 * 
 * @param out the destination ostream
 * @param node address of the node (sdf.addr())
 * @param fields list of fields (sdf.fields())
 * @param trailing_comma add one last comma to the list
 * @return true if no error was met
 * @return false errors were met
 */
bool fields2cpp (std::ostream& out, const void * node, fields_t fields, bool trailing_comma=true);

/**
 * @brief Apply an std::map into fields of an SDF node.
 * 
 * @param map 
 * @param node 
 * @param fields 
 * @return true 
 * @return false 
 */
bool map2fields (const std::map<std::string,std::string>& map, const void * node, fields_t fields);

/**
 * @brief Apply the content of an xml node onto an SDF node.
 * 
 * @param xml 
 * @param node 
 * @param fields 
 * @return true 
 * @return false 
 */
bool xml2fields (const pugi::xml_node& xml, const void * node, fields_t fields);


template<sdf::attrs_i T>
bool attrs2cpp(const typename T::extras_t& attrs, std::ostream& out){
    throw "NotImplemented";
    return false;
}

template<> 
bool attrs2cpp<sdf::idx_attrs<>>(const sdf::idx_attrs<>::extras_t& attrs, std::ostream& out){
    out<<attrs.uid<<","<<attrs.gid<<","<<attrs.idx<<","<<attrs.weak;
    return true;
}

template<> 
bool attrs2cpp<sdf::color_attrs>(const sdf::color_attrs::extras_t& attrs, std::ostream& out){
    out<<attrs.r<<","<<attrs.g<<","<<attrs.b<<","<<attrs.a;
    return true;
}

//TODO: implement missing cases
template<sdf::attrs_i T>
bool attrs2xml(const typename T::extras_t& attrs, pugi::xml_node& out){
    throw "NotImplemented";
    return false;
}

template<>
bool attrs2xml<sdf::idx_attrs<>>(const sdf::idx_attrs<>::extras_t& attrs, pugi::xml_node& out){
    out.append_attribute("uid").set_value(attrs.uid);
    out.append_attribute("gid").set_value(attrs.gid);
    out.append_attribute("idx").set_value(attrs.idx);
    out.append_attribute("weak").set_value((bool)attrs.weak);
    return true;
}

template<sdf::attrs_i T>
bool xml2attrs(const pugi::xml_node& in, typename T::extras_t& attrs, const typename T::extras_t& defval = {}){
    throw "NotImplemented";
    return false;
}

template<>
bool xml2attrs<sdf::idx_attrs<>>(const pugi::xml_node& in, sdf::idx_attrs<>::extras_t& attrs, const typename sdf::idx_attrs<>::extras_t& defval){
    static size_t uid = 0;

    //TODO: Add checks
    if(strcmp(in.attribute("uid").as_string(""),"*")==0)attrs.uid=++uid;
    else attrs.uid = in.attribute("uid").as_uint(0);
    attrs.gid = in.attribute("gid").as_uint(defval.gid);
    attrs.idx = in.attribute("idx").as_uint(defval.idx);
    attrs.weak = in.attribute("weak").as_bool(defval.weak);
    return true;
}

/**
 * @brief Compile an SDF into C++ code
 * 
 * @param usdf the referenced sdf
 * @param out the destination ostream
 * @return true 
 * @return false 
 */
bool sdf2cpp (sdf::sdf_i auto& usdf, std::ostream& out){
    struct entry_t{
        const char* tag;
        sdf::fields_t fields;
        const void* base;
        size_t children;
        size_t current;
    };
    std::vector<entry_t> depth = {};

    static auto op = [&](const char* tag, sdf::fields_t fields, const void* base, size_t children)->bool{
        //Handle comma separator for operator args
        if(!depth.empty()){
            auto& entry = depth.back();
            if(entry.current+1!=entry.children)out << ",";
        }

        //Leaf
        if(children==0){
            out<<tag<<"({";
            fields2cpp(out, base, fields);
            out << "{";
            using attrs_t = std::remove_reference_t<decltype(usdf)>::attrs_t;
            //This works because by construction of the layout, the cfg of leaves is always at the beginning of the object.
            attrs2cpp<attrs_t>((*(const typename attrs_t::extras_t*)base),out);
            out << "}}";
            out <<")";

            if(depth.empty())return true;
        }
        //Operator
        else{
            out<<tag<<"(";
            depth.push_back({tag,fields,base,children,children});
        }

        //Escape nesting
        while(!depth.empty()){
            auto& entry = depth.back();
            if(entry.current==0){
                out<<",{";
                fields2cpp(out, entry.base, entry.fields,false);
                out<<"})";
                depth.pop_back();
            }
            else{
                entry.current--;
                break;
            }
        }

        return true;
    };

    return usdf.ctree_visit_pre(op);
};



}

}
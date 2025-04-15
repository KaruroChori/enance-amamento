#pragma once

/**
 * @file xml.hpp
 * @author karurochari
 * @brief Implementation of serialization/deserialization as XML for SDF scenes
 * @date 2025-03-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */

/*
TODO: 
- Add support for associative region Join/Xor (and later associative smooth versions)
- Add support for Forward (based on the forest label assigned to each entry)
*/

#define GLM_FORCE_INLINE
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <memory>
#include <pugixml.hpp>

#include <cstring>
#include <string>
#include <utility>
#include <charconv>
#include <string_view>
#include <stack>
#include <format>

#include <ui/ui.hpp>

#include "sdf/sdf.hpp"

// Helper function that converts a character to lowercase on compile time
constexpr char charToLower(const char c) {
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

// Our compile time string class that is used to pass around the converted string
template <std::size_t N>
class const_str {
private:
    const char s[N+1]; // One extra byte to fill with a 0 value

public:
    // Constructor that is given the char array and an integer sequence to use parameter pack expansion on the array
    template <typename T, T... Nums>
    constexpr const_str(const char (&str)[N], std::integer_sequence<T, Nums...>)
        : s{charToLower(str[Nums])..., 0} {
    }

    // Compile time access operator to the characters
    constexpr char operator[] (std::size_t i) const {
        return s[i];
    }

    // Get a pointer to the array at runtime. Even though this happens at runtime, this is a fast operation and much faster than the actual conversion
    operator const char*() const {
        return s;
    }
};


// The code that we are actually going to call
template <std::size_t N>
constexpr const_str<N> toLower(const char(&str)[N]) {
    return {str, std::make_integer_sequence<unsigned, N>()};
}

static std::vector<std::string_view> split_string (const char* str, char delim) {
    if(str==nullptr)return {};
    std::vector<std::string_view> result;
    size_t i = 0, last = 0;
    for(;; i++){
        if(str[i]==delim){result.emplace_back(str+last,i-last);i++;last=i;}
        if(str[i]==0){result.emplace_back(str+last,i-last);break;}
    }
    return result;
}

template<typename Attrs>
struct parse_xml{
    private:

    std::stack<typename Attrs::extras_t> tmp_extras;
    std::map<std::string,uint64_t> named;   //Any named entity, regardless of nesting
    std::map<uint64_t,std::shared_ptr<sdf::utils::base_dyn<Attrs>>,std::less<void>> index;   //Any entity
    std::map<std::string,std::shared_ptr<sdf::utils::base_dyn<Attrs>>,std::less<void>> forest;  //Only top levels

    std::string root_label;

    App::treeview_t ui_tree;
    uint64_t next_uid=0;

    struct nodes_t{
        std::shared_ptr<sdf::utils::base_dyn<Attrs>> sdf;
        App::treeview_t::entry_t ui;
    };

    void warning(const char* str){printf("WARNING: %s\n",str);}
    void error(const char* str){printf("ERROR:  %s\n",str);throw "Error";}

    template<typename T, uint N>
    void handle_field(const pugi::xml_node& root, const sdf::field_t& field, uint8_t* base){ 
        T tmp[N];
        auto str = split_string(root.attribute(field.name).as_string(nullptr),'|');
        
        {
            auto tk_0 = root.attribute((std::string(field.name)+".0").c_str()).as_string(nullptr);
            auto tk_x = root.attribute((std::string(field.name)+".x").c_str()).as_string(nullptr);
            auto tk_r = root.attribute((std::string(field.name)+".r").c_str()).as_string(nullptr);
            auto tk_u = root.attribute((std::string(field.name)+".u").c_str()).as_string(nullptr);

            auto tk = !tk_0?tk_0:(!tk_x?tk_x:(!tk_r?tk_r:(tk_u)));
            if(tk!=nullptr){str[0]=tk;}
        }

        {
            auto tk_0 = root.attribute((std::string(field.name)+".1").c_str()).as_string(nullptr);
            auto tk_x = root.attribute((std::string(field.name)+".y").c_str()).as_string(nullptr);
            auto tk_r = root.attribute((std::string(field.name)+".g").c_str()).as_string(nullptr);
            auto tk_u = root.attribute((std::string(field.name)+".v").c_str()).as_string(nullptr);

            auto tk = !tk_0?tk_0:(!tk_x?tk_x:(!tk_r?tk_r:(tk_u)));
            if(tk!=nullptr){str[1]=tk;}
        }

        {
            auto tk_0 = root.attribute((std::string(field.name)+".2").c_str()).as_string(nullptr);
            auto tk_x = root.attribute((std::string(field.name)+".z").c_str()).as_string(nullptr);
            auto tk_r = root.attribute((std::string(field.name)+".b").c_str()).as_string(nullptr);

            auto tk = !tk_0?tk_0:(!tk_x?tk_x:(tk_r));
            if(tk!=nullptr){str[2]=tk;}
        }

        {
            auto tk_0 = root.attribute((std::string(field.name)+".3").c_str()).as_string(nullptr);
            auto tk_x = root.attribute((std::string(field.name)+".w").c_str()).as_string(nullptr);
            auto tk_r = root.attribute((std::string(field.name)+".a").c_str()).as_string(nullptr);

            auto tk = !tk_0?tk_0:(!tk_x?tk_x:(tk_r));
            if(tk!=nullptr){str[3]=tk;}
        }

        if(str.size()!=0){
            if(str.size()!=N)warning(std::format("Size of input {} not matching dimensionality. Modulo wrapping",str.size()).c_str());
            for(uint n=0;n<N;n++){
                std::string_view segment = str[n%str.size()];
                std::from_chars(segment.begin(),segment.end(),*(tmp+n));
                if(field.min!=nullptr && tmp[n]<((T*)field.min)[n]){error("value low");}
                else if(field.max!=nullptr && tmp[n]>((T*)field.max)[n]){error("value high");}
                else if(field.validate!=nullptr && !field.validate(&tmp)){error("validation failed");}
            }
        }
        else if(field.defval!=nullptr){
            for(uint n=0;n<N;n++)tmp[n]=((T*)field.defval)[n];
        }
        else {error("Unable to set argument.");}

        //Finally apply values
        memcpy(base+field.offset,tmp,field.length);
    }

    template<bool IS_OPERATOR>
    void parse_attrs(const pugi::xml_node& root, uint8_t* base, const sdf::field_t fields[], size_t fields_i){
        //Assumption: cfg is at the root of the object by construction.
        if constexpr(!IS_OPERATOR){
            typename Attrs::extras_t* cfg = (typename Attrs::extras_t*)base;
            auto cfg_node=root.child("cfg");
            cfg->from_xml((const sdf::xml&)cfg_node,tmp_extras.top());
        }

        for(size_t i=0;i<fields_i;i++){
            printf(">> %d--%s\n",i,fields[i].name);
            auto& field = fields[i];
            switch(field.type){
                case sdf::field_t::type_unknown:
                    //Skip, not supported
                    break;
                case sdf::field_t::type_cfg:
                    //Separate handling
                    break;
                case sdf::field_t::type_float:
                    this->handle_field<float,1>(root,field,base);
                    break;
                case sdf::field_t::type_vec2:
                    this->handle_field<float,2>(root,field,base);
                    break;
                case sdf::field_t::type_vec3:
                    this->handle_field<float,3>(root,field,base);
                    break;
                case sdf::field_t::type_int:
                    this->handle_field<int,1>(root,field,base);
                    break;
                case sdf::field_t::type_ivec2:
                    this->handle_field<int,2>(root,field,base);
                    break;
                case sdf::field_t::type_ivec3:
                    this->handle_field<int,3>(root,field,base);
                    break;
                case sdf::field_t::type_bool:
                    //TODO: Support true and false
                    this->handle_field<int,1>(root,field,base);
                    break;
                case sdf::field_t::type_tribool:
                {
                    //TODO: temporary implementation before triboolean are supported.
                    this->handle_field<int,1>(root,field,base);
                    break;
                }
                case sdf::field_t::type_enum:
                    //TODO: temporary implementation before enum entries are supported.
                    this->handle_field<int,1>(root,field,base);
                    break;
                case sdf::field_t::type_shared_buffer:
                    //TODO: Support true and false
                    this->handle_field<size_t,1>(root,field,base);
                    break;
                break;
            }
        }
    }

    #define XML_PRIMITIVE(NAME) \
        else if(strcmp(root.name(),toLower(#NAME))==0){\
            base.sdf = sdf::dynamic::NAME({});\
            auto real_base = (uint8_t*) &(dynamic_cast<sdf::impl_base::NAME<Attrs>*>(dynamic_cast<sdf::dynamic::NAME##_t<Attrs>*>(&*(base.sdf)))->cfg);\
            \
            parse_attrs<false>(root,real_base,sdf::dynamic::NAME##_t<Attrs>::_fields,sizeof(sdf::dynamic::NAME##_t<Attrs>::_fields)/sizeof(sdf::field_t));\
        }

    #define XML_OP2(NAME)\
        else if(strcmp(root.name(),toLower(#NAME))==0){\
            auto first_child =  root.first_child();\
            auto second_child = first_child.next_sibling();\
            \
            nodes_t left = parse_node(first_child);\
            nodes_t right = parse_node(second_child);\
            \
            base.sdf = sdf::dynamic::NAME(left.sdf,right.sdf);\
            base.ui.children = {left.ui, right.ui};\
            \
            auto real_base = (uint8_t*) &((dynamic_cast<sdf::utils::dyn_op<Attrs,sdf::impl::NAME<decltype(left.sdf),decltype(right.sdf)>>::operation*>(&*(base.sdf)))->cfg);\
            \
            parse_attrs<true>(root,real_base,sdf::dynamic::NAME##_t<Attrs>::_fields,sizeof(sdf::dynamic::NAME##_t<Attrs>::_fields)/sizeof(sdf::field_t));\
            if(second_child!=root.last_child()){\
                warning("Residual children in binary operator detected");\
            }\
        }

    #define XML_OP1(NAME)\
        else if(strcmp(root.name(),toLower(#NAME))==0){\
            auto first_child =  root.first_child();\
            \
            nodes_t left = parse_node(first_child);\
            \
            base.sdf = sdf::dynamic::NAME(left.sdf);\
            base.ui.children = {left.ui};\
            \
            auto real_base = (uint8_t*) &((dynamic_cast<sdf::utils::dyn_op<Attrs,sdf::impl::NAME<decltype(left.sdf)>>::operation*>(&*(base.sdf)))->cfg);\
            \
            parse_attrs<true>(root,real_base,sdf::dynamic::NAME##_t<Attrs>::_fields,sizeof(sdf::dynamic::NAME##_t<Attrs>::_fields)/sizeof(sdf::field_t));\
            if(first_child!=root.last_child()){\
                warning("Residual children in binary operator detected");\
            }\
        }

    void parse_forest(const pugi::xml_node& root){
        if(strcmp(root.name(),"forest")!=0){error("No forest root found.");}
        for(auto& child: root.children()){
            auto label = child.attribute("label").as_string(nullptr);
            if(label==nullptr)error("Trees must have a label");

            //sdf_tree
            nodes_t node_ref = parse_node(child);
            forest.emplace(std::string(label),node_ref.sdf);

            //ui_tree
            ui_tree.children.push_back(node_ref.ui);
        }
    }

    nodes_t parse_node(const pugi::xml_node& root){
        nodes_t base;
        if(false){}
        XML_PRIMITIVE(Sphere)
        XML_PRIMITIVE(Box)
        XML_PRIMITIVE(Plane)
        XML_PRIMITIVE(Zero)

        XML_OP2(Join)
        XML_OP2(Cut)
        XML_OP2(Common)
        XML_OP2(Xor)

        XML_OP2(SmoothJoin)

        XML_OP1(Translate)
        XML_OP1(Rotate)
        XML_OP1(Scale)
        //XML_OP1(Material) Disabled for now, support for shared_ptr needed.

        else if(strcmp(root.name(),"group")==0){
            typename Attrs::extras_t current;
            current.from_xml((const sdf::xml&)root,tmp_extras.top());
            tmp_extras.push(current);
            base = parse_node(root.first_child());
            if(root.first_child()!=root.last_child()){
                warning("Objects skipped as they were not part of any expression.");
            }
            tmp_extras.pop();
        }
        else if(strcmp(root.name(),"forward")==0){
            //Special handling for it. Check the attribute `ref` from the current map being built and use that.
        }
        else if(strcmp(root.name(),"associative")==0){
            //Convert a linear list of nodes into a binary tree with the join operator as inner nodes, and the actual original entries as leaves.
            auto type=root.attribute("mode").as_string("join");
            if(strcmp(type,"join")){

            }
            else{
                warning("Unknown associative block, skip");
            }
        }
        else{
            warning("Unknown entity, skip");
        }

        auto node_label = root.attribute("label").as_string(nullptr);
        next_uid++;
        base.ui.label=std::format("{}({})##{}",node_label!=nullptr?std::format("{} ",node_label):std::string(""), root.name(),next_uid);
        if(node_label!=nullptr)named.emplace(std::string(node_label),next_uid);
        index.emplace(next_uid,base.sdf);
        base.ui.ctx=next_uid;
        return base;
    }

    #undef XML_PRIMITIVE
    #undef XML_OP1
    #undef XML_OP2

    public:
        parse_xml(const pugi::xml_node& root){
            tmp_extras.push({});
            parse_forest(root);
            root_label = root.attribute("root").as_string("$");
        }

        std::shared_ptr<sdf::utils::base_dyn<Attrs>> sdf_root(){
            auto it = forest.find(std::string_view(root_label));
            if(it==forest.end())error("root not found");
            return it->second;
        }

        App::treeview_t& ui_root(){
            return ui_tree;
        }


        //TODO: Add compile to generate its C++ code.


};
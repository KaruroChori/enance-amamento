/**
 * @file xml-parser.cpp
 * @author karurochari
 * @brief 
 * @version 0.1
 * @date 2025-05-01
 * 
 * @copyright Copyright (c) 2025, licenced as CC-ND-NC-BY-SA for now
 * 
 */

#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <print>
#include <stack>
#include <string>
#include <string_view>
#include <expected>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>


namespace xml{
typedef std::ptrdiff_t delta_ptr_t ;

struct sv{
    delta_ptr_t  base;
    size_t       length;

    sv(delta_ptr_t b, size_t len):base(b),length(len){}
    sv(void* offset, std::string_view v){base=((uint8_t*)v.data()-(uint8_t*)offset);length=v.length();}

    sv()=delete;
};

struct node_t;
struct attr_t;
struct text_t;
struct comment_t;
struct cdata_t;
struct proc_t;
struct inject_t;
struct unknown_t;

struct Builder;
struct Tree;

enum struct feature_t{
    OK,
    NONE,
    ERROR,
    NOT_SUPPORTED,
    NOT_IMPLEMENTED,
};

enum struct type_t{
    UNKNOWN,
    NODE,
    ATTR,
    TEXT,
    COMMENT,
    PROC,
    CDATA,
    INJECT
};

namespace serialize{
    //https://stackoverflow.com/questions/1091945/what-characters-do-i-need-to-escape-in-xml-documents
    //An attempt will be made to keep the input string_view, unless escaping is needed.

    typedef std::optional<std::variant<std::string,std::string_view>> ret_t;

    constexpr std::optional<std::string_view> validate_xml_label(std::string_view str){
        int pos = 0;
        for(auto& c : str){
            if(pos==0 && (c=='_' or (c>'a' && c<'z') or (c>'A' && c<'Z'))){/*OK*/}
            //In theory some intervals of utf8 should be negated. But this filter is good enough for now.
            else if(pos!=0 && (c=='_' or c=='.' or c=='-' or (c>'0' && c<'9') or (c>'a' && c<'z') or (c>'A' && c<'Z') or (c>127))){/*OK*/}
            else{
                std::print("{} @ pos {}----- {} {} {} \n",(int)c, pos, str, (void*)str.data(), str.length());
                return {};
            }
            pos++;
        }
        return str;
    }

    constexpr ret_t to_xml_attr_1(std::string_view str){
        int rule_a = 0;
        for(auto& c : str){
            if(c=='<')rule_a+=sizeof("&lt;")-1-1;
            else if(c=='&')rule_a+=sizeof("&amp;")-1-1;
            else if(c=='\'')rule_a+=sizeof("&apos;")-1-1;
            //Maybe some config to enable the others as well even if not needed for a correct XML serialization?
        }
        if(rule_a==0)return str;
        else{
            std::string tmp;
            tmp.reserve(str.length()+rule_a);
            for(auto& c : str){
                if(c=='<')tmp+="&lt;";
                else if(c=='&')tmp+="&amp;";
                else if(c=='\'')tmp+="&apos;";
                else tmp+=c;
            }
            return tmp;
        }
    }

    constexpr ret_t to_xml_attr_2(std::string_view str){
        int rule_a = 0;
        for(auto& c : str){
            if(c=='<')rule_a+=sizeof("&lt;")-1-1;
            else if(c=='&')rule_a+=sizeof("&amp;")-1-1;
            else if(c=='"')rule_a+=sizeof("&quot;")-1-1;
            //Maybe some config to enable the others as well even if not needed for a correct XML serialization?
        }
        if(rule_a==0)return str;
        else{
            std::string tmp;
            tmp.reserve(str.length()+rule_a);
            for(auto& c : str){
                if(c=='<')tmp+="&lt;";
                else if(c=='&')tmp+="&amp;";
                else if(c=='"')tmp+="&quot;";
                else tmp+=c;
            }
            return tmp;
        }
    }

    constexpr ret_t to_xml_text(std::string_view str){
        int rule_a = 0;
        int rule_b = 0;
        for(auto& c : str){
            if(c=='<')rule_a+=sizeof("&lt;")-1-1;
            else if(c=='&')rule_a+=sizeof("&amp;")-1-1;
            if(rule_b==0 && c==']')rule_b=1;
            if(rule_b==1 && c==']')rule_b=2;
            if(rule_b==2 && c=='>')return {};   //]]> not allowed in text, it must be escaped.

            //Maybe some config to enable the others as well even if not needed for a correct XML serialization?
        }
        if(rule_a==0)return str;
        else{
            std::string tmp;
            tmp.reserve(str.length()+rule_a);
            for(auto& c : str){
                if(c=='<')tmp+="&lt;";
                else if(c=='&')tmp+="&amp;";
                else tmp+=c;
            }
            return tmp;
        }
    }

    constexpr ret_t to_xml_cdata(std::string_view str){
        int rule_a = 0;
        for(auto& c : str){
            if(rule_a==0 && c==']')rule_a=1;
            else if(rule_a==1 && c==']')rule_a=2;
            else if(rule_a==2 && c=='>')return {};   //Disallowed sequence ]]>
            else rule_a=0;
        }
        return str;
    }

    constexpr ret_t to_xml_comment(std::string_view str){
        int rule_a = 0;
        for(auto& c : str){
            if(rule_a==0 && c=='-')rule_a=1;
            else if(rule_a==1 && c=='-')return {};   //Disallowed sequence --
            else rule_a=0;
        }
        return str;
    }

    constexpr ret_t to_xml_proc(std::string_view str){
        int rule_a = 0;
        for(auto& c : str){
            if(rule_a==0 && c=='?')rule_a=1;
            else if(rule_a==1 && c=='>')return {};   //Disallowed sequence ?>
            else rule_a=0;
        }
        return str;
    }
}

template<typename T>
concept thing_i = requires(T self){
    {self.type()} -> std::same_as<type_t>;
    {self.ns()} -> std::same_as<std::expected<sv,feature_t>>;
    {self.name()} -> std::same_as<std::expected<sv,feature_t>>;
    {self.value()} -> std::same_as<std::expected<sv,feature_t>>;

    {self.children()} -> std::same_as<std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t>>;
    {self.attrs()} -> std::same_as<std::expected<std::pair<const attr_t*, const attr_t*>,feature_t>>;

    {self.parent()} -> std::same_as<const node_t*>;
    {self.prev()} -> std::same_as<const unknown_t*>;
    {self.next()} -> std::same_as<const unknown_t*>;

    {self.has_parent()} -> std::same_as<bool>;
    {self.has_prev()} -> std::same_as<bool>;
    {self.has_next()} -> std::same_as<bool>;

    {self.path()} -> std::same_as<std::string>;
};

template <typename T>
struct base_t{
    protected:
    type_t _type = T::deftype();

    public:
    typedef T base;

    constexpr inline type_t type() const {return _type;};

    constexpr inline std::expected<sv,feature_t> ns() const {return static_cast<const T*>(this)->ns();}
    constexpr inline std::expected<sv,feature_t> name() const {return static_cast<const T*>(this)->name();}
    constexpr inline std::expected<sv,feature_t> value() const {return static_cast<const T*>(this)->value();}

    constexpr inline std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t> children() const {return static_cast<const T*>(this)->children();}
    constexpr inline std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs() const {return static_cast<const T*>(this)->attrs();}

    constexpr inline const node_t* parent() const {return static_cast<const T*>(this)->parent();}
    constexpr inline const unknown_t* prev() const {return static_cast<const T*>(this)->prev();}
    constexpr inline const unknown_t* next() const {return static_cast<const T*>(this)->next();}

    constexpr inline bool has_parent() const {return static_cast<const T*>(this)->has_parent();}
    constexpr inline bool has_prev() const {return static_cast<const T*>(this)->has_prev();}
    constexpr inline bool has_next() const {return static_cast<const T*>(this)->has_next();}

    constexpr inline std::string path() const {
        return std::format("{}/{}",parent()!=nullptr?parent()->path():"",static_cast<const T*>(this)->path_h());
    }

    constexpr auto children_fwd() const;
    constexpr auto attrs_fwd() const;

    friend Builder;
    friend Tree;
};


struct attr_t{
    private:
    sv _ns;
    sv _name;
    sv _value;

    public:

    constexpr inline attr_t(void* offset, std::string_view _ns, std::string_view _name, std::string_view _value):
        _ns(offset,*serialize::validate_xml_label(_ns)),
        _name(offset,*serialize::validate_xml_label(_name)),
        _value(offset,_value){}

    constexpr inline std::expected<sv,feature_t> ns() const {return _ns;}
    constexpr inline std::expected<sv,feature_t> name() const {return _name;}
    constexpr inline std::expected<sv,feature_t> value() const {return _value;}
    
};


struct node_t : base_t<node_t>{
    private:
    delta_ptr_t _parent;
    delta_ptr_t _prev;
    delta_ptr_t _next;

    delta_ptr_t _size;

    size_t      attrs_count;

    sv _ns;
    sv _name;

    constexpr inline node_t(void* offset, node_t* _parent, std::string_view _ns, std::string_view _name):
        _parent((uint8_t*)_parent-(uint8_t*)this),
        _ns(offset,*serialize::validate_xml_label(_ns)),
        _name(offset,*serialize::validate_xml_label(_name))
    {
        _size=0;
        attrs_count=0;
    }

    constexpr inline void set_parent(node_t* parent){_parent=(uint8_t*)parent-(uint8_t*)this;}
    constexpr inline void set_prev(unknown_t* prev){_prev=(uint8_t*)prev-(uint8_t*)this;}
    constexpr inline void set_next(unknown_t* next){_next=(uint8_t*)next-(uint8_t*)this;}

    public:

    using base_t::type;
    
    constexpr static inline type_t deftype() {return type_t::NODE;};
    constexpr inline std::expected<sv,feature_t> ns() const {return _ns;}
    constexpr inline std::expected<sv,feature_t> name() const {return _name;}
    constexpr inline std::expected<sv,feature_t> value() const {return std::unexpected(feature_t::NOT_SUPPORTED);}

    constexpr inline std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t> children() const {
        return std::pair{
            (const unknown_t*)((const uint8_t*)this+sizeof(node_t)+sizeof(attr_t)*attrs_count),
            (const unknown_t*)((const uint8_t*)this+_size)
        };
    }
    constexpr inline std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs() const {
        return std::pair{
            (const attr_t*)((const uint8_t*)this+sizeof(node_t)),
            (const attr_t*)((const uint8_t*)this+sizeof(node_t)+sizeof(attr_t)*attrs_count)
        };
    }

    constexpr inline const node_t* parent() const {
        if(_parent==0)return nullptr;
        return (const node_t*)((const uint8_t*)this+_parent);
    }
    constexpr inline const unknown_t* prev() const {
        if(_prev==0)return nullptr;  //TODO: check this one
        return (const unknown_t*)((const uint8_t*)this+_prev);
    }
    constexpr inline const unknown_t* next() const {
        if(_next==0)return (const unknown_t*) (parent()->_size+_parent+(const uint8_t*)this); 
        return (const unknown_t*)((const uint8_t*)this+_next);
    }

    constexpr inline bool has_parent() const {return _parent!=0;}
    constexpr inline bool has_prev() const {return _prev!=0;}
    constexpr inline bool has_next() const {return _next!=0;}

    /*
    constexpr inline std::string path_h() const {
        return std::format("{}{}{}", _ns, _ns==""?"":":", _name);
    }
    */
    
    friend Builder;
    friend Tree;
    friend unknown_t;
};


template<typename T>
struct leaf_t : base_t<T>{
    private:
    delta_ptr_t _parent;
    delta_ptr_t _prev;
    // delta_ptr_t _next; not needed. Can be statically determined by its size and the children information of the parent.

    sv _value;

    constexpr inline void set_parent(node_t* parent){_parent=(uint8_t*)parent-(uint8_t*)this;}
    constexpr inline void set_prev(unknown_t* prev){_prev=(uint8_t*)prev-(uint8_t*)this;}
    constexpr inline void set_next(unknown_t* next){/*_next=(uint8_t*)next-(uint8_t*)this;*/}


    protected:
    
    constexpr leaf_t(void* offset, std::string_view value):_value(offset,value){}

    public:

    using base_t<T>::type;

    constexpr inline std::expected<sv,feature_t> ns() const {return std::unexpected(feature_t::NOT_SUPPORTED);}
    constexpr inline std::expected<sv,feature_t> name() const {return std::unexpected(feature_t::NOT_SUPPORTED);}
    constexpr inline std::expected<sv,feature_t> value() const {return _value;}

    constexpr inline std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t> children() const {return std::unexpected(feature_t::NOT_SUPPORTED);}
    constexpr inline std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs() const {return std::unexpected(feature_t::NOT_SUPPORTED);}

    constexpr inline const node_t* parent() const {return (const node_t*)((const uint8_t*)this+_parent);}
    constexpr inline const unknown_t*prev() const {return (const unknown_t*)((const uint8_t*)this+_prev);}
    constexpr inline const unknown_t* next() const {return (const unknown_t*)((const uint8_t*)this+sizeof(leaf_t));}

    constexpr inline bool has_parent() const {return _parent!=0;}
    constexpr inline bool has_prev() const {return _prev!=0;}
    constexpr inline bool has_next() const {return has_parent() && (next()<(parent()->children())->second)!=0;}   //TODO:check

    friend Builder;
    friend Tree;
    friend unknown_t;
};

struct comment_t : leaf_t<comment_t>{
    comment_t(void* offset, std::string_view value):leaf_t(offset, value){}
    constexpr static inline type_t deftype() {return type_t::COMMENT;};

    constexpr inline std::string path_h() const { return std::format("#comment"); }

    friend Builder;
    friend Tree;
};

struct cdata_t : leaf_t<cdata_t>{
    cdata_t(void* offset, std::string_view value):leaf_t(offset, value){}
    constexpr static inline type_t deftype() {return type_t::CDATA;};

    constexpr inline std::string path_h() const { return std::format("#cdata"); }

    friend Builder;
    friend Tree;
};

struct text_t : leaf_t<text_t>{
    text_t(void* offset, std::string_view value):leaf_t(offset, value){}
    constexpr static inline type_t deftype() {return type_t::TEXT;};

    constexpr inline std::string path_h() const { return std::format("#text"); }
    
    friend Builder;
    friend Tree;
};

struct proc_t : leaf_t<proc_t>{
    proc_t(void* offset, std::string_view value):leaf_t(offset, value){}
    constexpr static inline type_t deftype() {return type_t::PROC;};

    constexpr inline std::string path_h() const { return std::format("#proc"); }

    friend Builder;
    friend Tree;
};

struct inject_t : leaf_t<inject_t>{
    inject_t(void* offset, std::string_view value):leaf_t(offset, value){}
    constexpr static inline type_t deftype() {return type_t::INJECT;};

    constexpr inline std::string path_h() const { return std::format("#leaf"); }

    friend Builder;
    friend Tree;
};

static_assert(thing_i<node_t>);
static_assert(thing_i<comment_t>);
static_assert(thing_i<cdata_t>);
static_assert(thing_i<text_t>);
static_assert(thing_i<proc_t>);
static_assert(thing_i<inject_t>);

#define DISPATCH(X,Y) \
if (type() == type_t::NODE) return ((node_t*)this) -> X;\
else if (type() == type_t::TEXT) return ((text_t*)this)-> X;\
else if (type() == type_t::COMMENT) return ((comment_t*)this)-> X;\
else if (type() == type_t::PROC) return ((proc_t*)this)-> X;\
else if (type() == type_t::CDATA) return ((cdata_t*)this)-> X;\
else if (type() == type_t::INJECT) return ((inject_t*)this)-> X;\
else{\
    Y;\
}\

#define CDISPATCH(X,Y) \
if (type() == type_t::NODE) return ((const node_t*)this) -> X;\
else if (type() == type_t::TEXT) return ((const text_t*)this)-> X;\
else if (type() == type_t::COMMENT) return ((const comment_t*)this)-> X;\
else if (type() == type_t::PROC) return ((const proc_t*)this)-> X;\
else if (type() == type_t::CDATA) return ((const cdata_t*)this)-> X;\
else if (type() == type_t::INJECT) return ((const inject_t*)this)-> X;\
else{\
    Y;\
}\



struct unknown_t : base_t<unknown_t>{
    private:

    constexpr void set_parent(node_t* parent){DISPATCH(set_parent(parent),std::terminate());}
    constexpr void set_prev(unknown_t* prev){DISPATCH(set_prev(prev),std::terminate());}
    constexpr void set_next(unknown_t* next){DISPATCH(set_next(next),std::terminate());}

    public:

    constexpr static inline type_t deftype() {return type_t::UNKNOWN;};

    constexpr std::expected<sv,feature_t> ns() const {CDISPATCH(ns(),std::terminate());}
    constexpr std::expected<sv,feature_t> name() const {CDISPATCH(name(),std::terminate());}
    constexpr std::expected<sv,feature_t> value() const {CDISPATCH(value(),std::terminate());}

    constexpr std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t> children() const {CDISPATCH(children(),std::terminate());}
    constexpr std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs() const {CDISPATCH(attrs(),std::terminate());}

    constexpr const node_t* parent() const {CDISPATCH(parent(),std::terminate());}
    constexpr const unknown_t* prev() const {CDISPATCH(prev(),std::terminate());}
    constexpr const unknown_t* next() const {CDISPATCH(next(),std::terminate());}

    constexpr inline bool has_parent() const {CDISPATCH(has_parent(),std::terminate());}
    constexpr inline bool has_prev() const {CDISPATCH(has_prev(),std::terminate());}
    constexpr inline bool has_next() const {CDISPATCH(has_next(),std::terminate());}

    friend Builder;
    friend Tree;
};


static_assert(thing_i<unknown_t>);

#undef DISPATCH
#undef CDISPATCH

struct node_iterator{
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = const unknown_t;
    using pointer           = const value_type*;
    using reference         = const value_type&;

    node_iterator(pointer ptr) : m_ptr(ptr) {}


    reference operator*() const { return *m_ptr; }
    pointer operator->() { return m_ptr; }

    node_iterator& operator++() { m_ptr=m_ptr->next(); return *this; }  
    node_iterator& operator--() { m_ptr=m_ptr->prev(); return *this; }  

    node_iterator operator++(int) { node_iterator tmp = *this; ++(*this); return tmp; }
    node_iterator operator--(int) { node_iterator tmp = *this; --(*this); return tmp; }

    friend bool operator== (const node_iterator& a, const node_iterator& b) { return a.m_ptr == b.m_ptr; };
    friend bool operator!= (const node_iterator& a, const node_iterator& b) { return a.m_ptr != b.m_ptr; };  


    private:
    pointer m_ptr;
};

struct attr_iterator{
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = const attr_t;
    using pointer           = const value_type*;
    using reference         = const value_type&;

    attr_iterator(pointer ptr) : m_ptr(ptr) {}


    reference operator*() const { return *m_ptr; }
    pointer operator->() { return m_ptr; }

    attr_iterator& operator++() { m_ptr++; return *this; }  
    attr_iterator& operator--() { m_ptr--; return *this; }  

    attr_iterator operator++(int) { attr_iterator tmp = *this; ++(*this); return tmp; }
    attr_iterator operator--(int) { attr_iterator tmp = *this; --(*this); return tmp; }

    friend bool operator== (const attr_iterator& a, const attr_iterator& b) { return a.m_ptr == b.m_ptr; };
    friend bool operator!= (const attr_iterator& a, const attr_iterator& b) { return a.m_ptr != b.m_ptr; };  


    private:
    pointer m_ptr;
};

template <typename T>
constexpr auto base_t<T>::children_fwd() const{

    struct self{
        node_iterator begin() const {return (*base.children()).first;}
        node_iterator end() const {return (*base.children()).second;}

        self(const base_t& b):base(b){}

        private:
            const base_t& base;
    };

    return self(*this);
}

template <typename T>
constexpr auto base_t<T>::attrs_fwd() const{

    struct self{
        attr_iterator begin() const {return (*base.attrs()).first;}
        attr_iterator end() const {return (*base.attrs()).second;}

        self(const base_t& b):base(b){}

        private:
            const base_t& base;
    };

    return self(*this);
}

struct Tree{
    std::vector<uint8_t> buffer;
    std::vector<uint8_t> symbols;
    const node_t& root;
    void* label_offset;

    Tree(std::vector<uint8_t>&& src):buffer(src),symbols({}),root(*(node_t*)src.data()),label_offset(nullptr){}

    Tree(std::vector<uint8_t>&& src, std::vector<uint8_t>&& sym):buffer(src),symbols(sym),root(*(node_t*)src.data()),label_offset(symbols.data()){}

    /**
     * @brief Reorder (in-place) children of a node based on a custom ordering criterion.
     * 
     * @param ref Node whose children are to be ordered
     * @param fn Criterion to determine the order of nodes. -1 for left<right, 0 for equals, 1 for left>right
     * @return true if the operation is successful
     */
    bool reorder_children(const node_t* ref, const std::function<int(const unknown_t*, const unknown_t*)>& fn){return false;}

    /**
     * @brief Reorder (in-place) attributes of a node based on a custom ordering criterion.
     * 
     * @param ref Node whose attributes are to be ordered
     * @param fn Criterion to determine the order of nodes. -1 for left<right, 0 for equals, 1 for left>right
     * @return true if the operation is successful
     */
    bool reorder(const node_t* ref, const std::function<int(const attr_t*, const attr_t*)>& fn){return false;}

    /**
     * @brief 
     * 
     * @param ref The node/leaf where to expand.
     * @param start 
     * @param end 
     * @return true 
     * @return false 
     */
    bool inject(const unknown_t* ref, const unknown_t* start, const unknown_t* end);

    bool inject_before(const unknown_t* ref, const unknown_t* start, const unknown_t* end);
    bool inject_after(const unknown_t* ref, const unknown_t* start, const unknown_t* end);

    /**
     * @brief Return a perfect deep copy of the current tree structure.
     * 
     * @return tree 
     */
    Tree clone() const;

    struct print_cfg_t{};

    inline bool print(std::ostream& out, const print_cfg_t& cfg = {})const{
        return print_h(out, cfg, (const unknown_t*)&root);
    }

    inline std::string_view rsv(sv s) const{
        return std::string_view(s.base+(char*)label_offset,s.base+(char*)label_offset+s.length);
    }

    private:
    //TODO: at some point, convert it not to be recursive.
    bool print_h(std::ostream& out, const print_cfg_t& cfg = {}, const unknown_t* ptr=nullptr) const{
        if(ptr->type()==type_t::NODE){
            if(ptr->children()->first==ptr->children()->second){
                out << std::format("<{}{}{}", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
                for(auto& i : ptr->attrs_fwd()){
                    auto t = serialize::to_xml_attr_2(rsv(*i.value()));
                    if(!t.has_value()){/*TODO: Error*/}
                    auto tt = t.value_or(std::string_view(""));
                    std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
                    out << std::format(" {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), sv);
                }
                out << "/>";
            }
            else{
                out << std::format("<{}{}{}", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
                for(auto& i : ptr->attrs_fwd()){
                    auto t = serialize::to_xml_attr_2(rsv(*i.value()));
                    if(!t.has_value()){/*TODO: Error*/}
                    auto tt = t.value_or(std::string_view(""));
                    std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
                    out << std::format(" {}{}{}=\"{}\"", rsv(*i.ns()), rsv(*i.ns())==""?"":":", rsv(*i.name()), sv);
                }
                out << ">";
                for(auto& i : ptr->children_fwd()){
                    print_h(out,cfg,&i);
                }
                out << std::format("</{}{}{}>", rsv(*ptr->ns()), rsv(*ptr->ns())==""?"":":", rsv(*ptr->name()));
            }
        }
        else if(ptr->type()==type_t::CDATA){
            auto t = serialize::to_xml_cdata(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            out << std::format("<![CDATA[{}]]>",sv);
        }
        else if(ptr->type()==type_t::COMMENT){
            auto t = serialize::to_xml_comment(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            out << std::format("<!--{}-->",sv);
        }
        else if(ptr->type()==type_t::TEXT){
            auto t = serialize::to_xml_text(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            out << std::format("{}",sv);
        }
        else if(ptr->type()==type_t::PROC){
            auto t = serialize::to_xml_proc(rsv(*ptr->value()));
            if(!t.has_value()){/*TODO: Error*/}
            auto tt = t.value_or(std::string_view(""));
            std::string_view sv = std::holds_alternative<std::string>(tt)?std::get<std::string>(tt):std::get<std::string_view>(tt);
            out << std::format("<?{}?>",sv);
        }
        else if(ptr->type()==type_t::INJECT){
            //Skip, injection points are not XML, they are only internally used.
        }
        return false;
    };
};

/**
 * @brief Helper class to build an XML tree via commands.
 * 
 */
struct Builder{
    enum struct error_t{
        OK,
        TREE_CLOSED,
        TREE_ATTR_CLOSED,
        STACK_EMPTY,
        MISFORMED,
    };

    private:
    std::vector<uint8_t> buffer;
    bool open = true;
    bool attribute_block = false;   //True after a begin to add attributes. It is automatically closed when any other command is triggered.
    
    std::stack<std::pair<ptrdiff_t,ptrdiff_t>> stack;

    template<typename T>
    error_t leaf(std::string_view value){
        if(open==false)return error_t::TREE_CLOSED;
        attribute_block=false;

        buffer.resize(buffer.size()+sizeof(T));

        auto& old_ctx = stack.top();

        unknown_t* prev = old_ctx.second!=-1?(unknown_t*)(buffer.data()+old_ctx.second):nullptr;

        T* tmp_node = new ((node_t*) & (uint8_t&) *( buffer.end()-sizeof(T) )) T(label_offset,value);

        if(prev!=nullptr){
            prev->set_next((unknown_t*)tmp_node);
            tmp_node->set_prev(prev);
        }
        old_ctx.second = (uint8_t*)tmp_node-buffer.data();

        return error_t::OK;
    }

    protected:
    void* label_offset = nullptr;

    std::expected<Tree,error_t> close(std::vector<uint8_t>&& symbols){
        if(open==false)return std::unexpected(error_t::TREE_CLOSED);
        open=false;
        if(stack.size()!=1)return std::unexpected(error_t::MISFORMED);
        stack.pop();
        return Tree(std::move(buffer),std::move(symbols));
    }

    public:

    Builder(){
        stack.push({0,-1});
    }

    error_t begin(std::string_view name, std::string_view ns=""){
        if(open==false)return error_t::TREE_CLOSED;

        buffer.resize(buffer.size()+sizeof(node_t));

        auto& old_ctx = stack.top();

        node_t* parent = (node_t*)(buffer.data()+old_ctx.first);
        unknown_t* prev = old_ctx.second!=-1?(unknown_t*)(buffer.data()+old_ctx.second):nullptr;

        //Emplace node
        node_t* tmp_node = new ((node_t*) & (uint8_t&) *( buffer.end()-sizeof(node_t) )) node_t(label_offset,parent,ns,name);

        if(prev!=nullptr){
            prev->set_next((unknown_t*)tmp_node);
            tmp_node->set_prev(prev);
        }
        old_ctx.second = (uint8_t*)tmp_node-buffer.data();

        stack.push({((uint8_t*)tmp_node-(uint8_t*)buffer.data()),-1});
        attribute_block=true;

        return error_t::OK;
    }

    error_t end(){
        if(open==false)return error_t::TREE_CLOSED;
        if(stack.size()<=1)return error_t::STACK_EMPTY;

        attribute_block=false;

        auto& ctx = stack.top();
        node_t* parent = (node_t*)(buffer.data()+ctx.first);
        parent->_size=buffer.size()-ctx.first;

        stack.pop();

        return error_t::OK;
    }

    error_t attr(std::string_view name, std::string_view value, std::string_view ns=""){
        if(open==false)return error_t::TREE_CLOSED;
        if(attribute_block==false)return error_t::TREE_ATTR_CLOSED;

        buffer.resize(buffer.size()+sizeof(attr_t));

        auto& old_ctx = stack.top();

        node_t* parent = (node_t*)(buffer.data()+old_ctx.first);

        //Emplace node
        new ((attr_t*) & (uint8_t&) *( buffer.end()-sizeof(attr_t) )) attr_t(label_offset,ns,name,value);

        parent->attrs_count++;

        return error_t::OK;
    }

 
    inline error_t text(std::string_view value){return leaf<text_t>(value);}
    inline error_t comment(std::string_view value){return leaf<comment_t>(value);}
    inline error_t cdata(std::string_view value){return leaf<cdata_t>(value);}
    inline error_t proc(std::string_view value){return leaf<proc_t>(value);}
    inline error_t inject(std::string_view value){return leaf<inject_t>(value);}

    std::expected<Tree,error_t> close(){
        if(open==false)return std::unexpected(error_t::TREE_CLOSED);
        open=false;
        if(stack.size()!=1)return std::unexpected(error_t::MISFORMED);
        stack.pop();
        return Tree(std::move(buffer),{});
    }

};

/**
 * @brief Wrapper of `Builder` to compress labels before usage, reducing the final memory footprint.
 * 
 */
struct BuilderCompressed : protected Builder{
    private:
    std::unordered_set<sv, std::function<uint64_t(sv)>,std::function<bool(sv, sv)>> idx_symbols;
    std::vector<uint8_t> symbols;

    /**
     * @brief 
     * NOTICE! The generated string_views have a very narrow lifetime. As symbols changes they are done.
     * Do not let them survive beyon the next change to symbols!
     * @param s 
     * @return std::string_view 
     */
    inline std::string_view rsv(sv s){
        return std::string_view(s.base+(char*)label_offset,s.base+(char*)label_offset+s.length);
    }
  
    sv symbol(std::string_view s){
        if(s.length()==0)return {0,0};

        auto it = idx_symbols.find(sv(label_offset,s));

        if(it==idx_symbols.end()){
            symbols.insert(symbols.end(),s.begin(),s.end());
            label_offset=symbols.data();    //Keeep this updated so that when sv are used they are always relative.

            sv ret(symbols.size()-s.length(),s.length());
            auto it = idx_symbols.insert(ret); 
            if(it.second==false)throw "NOOOOO";
            return *it.first;
        }
        else{
            return *it;
        }
    }

    public:



    inline BuilderCompressed():idx_symbols(64,
        [this](sv a){return std::hash<std::string_view>{}(rsv(a));},
        [this](sv a, sv b){return rsv(a)==rsv(b);}),symbols(){
            label_offset=symbols.data(); 
    }

    inline error_t begin(std::string_view name, std::string_view ns=""){
        auto a =symbol(name), b = symbol(ns);
        return Builder::begin(rsv(a),rsv(b));
    }
    inline error_t end(){
        return Builder::end();
    }
    inline error_t attr(std::string_view name, std::string_view value, std::string_view ns=""){
        auto a =symbol(name), b = symbol(value), c = symbol(ns);
        return Builder::attr(rsv(a),rsv(b),rsv(c));
    }
 
    inline error_t text(std::string_view value){
        return Builder::text(rsv( symbol(value)));
    }
    inline error_t comment(std::string_view value){
        return Builder::comment(rsv( symbol(value)));
    }
    inline error_t cdata(std::string_view value){
        return Builder::cdata(rsv( symbol(value)));
    }
    inline error_t proc(std::string_view value){
        return Builder::proc(rsv( symbol(value)));
    }
    inline error_t inject(std::string_view value){
        return Builder::inject(rsv( symbol(value)));
    }

    std::expected<Tree,error_t> close(){
        idx_symbols.clear();
        return Builder::close(std::move(symbols));
    }
};

struct Parser {};


struct ParserCompressed : protected Parser{

};

}


int main(){
    xml::BuilderCompressed build;

    build.begin("hello");
        build.attr("op1", "v'>&al1");
        build.attr("op2", "val1", "w");
        build.attr("op3", "val\"1");
        build.begin("hello1","s");
        build.end();
        build.begin("hello2","s");
            build.text("Banana <hello ciao=\"worldo\" &amp; &></world>"); 
        build.end();
        build.begin("hello3","s");
            build.begin("hello5","s");
            build.attr("op1", "val1");
            build.attr("op2", "val1");
            build.attr("op3", "val1");
            build.cdata("Hello'''''&&&& world!");
            build.end();
        build.end();
        build.begin("hello4","s");
        
        build.end();
    build.end();
    auto tree = *build.close();
    auto limits = *tree.root.children();
    std::print("{} {}\n",tree.rsv(*tree.root.name()),(uint8_t*)limits.second-(uint8_t*)limits.first);
    /*for(auto i = limits.first;i<limits.second;i=*i->next()){
        std::print("{}, {}\n",(long)i,(long)limits.second);
        std::print("{} \n",*i->name());
        if(!i->next().has_value())break;
    }
    for(auto& i : tree.root.attrs_fwd()){
        std::print("{} \n",*i.name());
    }
    for(auto& i : tree.root.children_fwd()){
        std::print("{} \n",*i.name());
        for(auto& j : i.children_fwd()){
            //std::print("{} \n",*j.name());
        }
    }
    */
    std::print("{},{},{}\n",sizeof(std::variant<std::string,std::string_view>),sizeof(std::string),sizeof(std::string_view));

    tree.print(std::cout,{});

    return 0;
}
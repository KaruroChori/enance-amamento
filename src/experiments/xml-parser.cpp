

#include <cassert>
#include <concepts>
#include <cstddef>
#include <print>
#include <string_view>
#include <expected>




namespace xml{
typedef std::ptrdiff_t delta_ptr_t ;

struct guard_t{
    bool used = false;
    struct guard_handle_t{
        guard_t& ref;

        guard_handle_t(guard_t& ref):ref(ref){
            assert(ref.used==false);
            ref.used=true;
        }
        ~guard_handle_t(){
            ref.used=false;
        }
    };
    guard_handle_t use(){return guard_handle_t(*this);}
};

struct node_t;
struct attr_t;
struct text_t;
struct comment_t;
struct cdata_t;
struct proc_t;
struct inject_t;
struct unknown_t;

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

template<typename T>
concept thing_i = requires(T self){
    {self.type()} -> std::same_as<type_t>;
    {self.ns()} -> std::same_as<std::expected<std::string_view,feature_t>>;
    {self.name()} -> std::same_as<std::expected<std::string_view,feature_t>>;
    {self.value()} -> std::same_as<std::expected<std::string_view,feature_t>>;

    {self.children()} -> std::same_as<std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t>>;
    {self.attrs()} -> std::same_as<std::expected<std::pair<const attr_t*, const attr_t*>,feature_t>>;

    {self.parent()} -> std::same_as<std::expected<const node_t*,feature_t>>;
    {self.prev()} -> std::same_as<std::expected<const unknown_t*,feature_t>>;
    {self.next()} -> std::same_as<std::expected<const unknown_t*,feature_t>>;
};

template <typename T>
struct base_t{
    protected:
    type_t _type = T::deftype();

    public:
    typedef T base;

    constexpr inline type_t type() const {return _type;};

    constexpr std::expected<std::string_view,feature_t> ns() const {return static_cast<const T*>(this)->ns();}
    constexpr std::expected<std::string_view,feature_t> name() const {return static_cast<const T*>(this)->name();}
    constexpr std::expected<std::string_view,feature_t> value() const {return static_cast<const T*>(this)->value();}

    constexpr std::expected<std::pair<const unknown_t*, const unknown_t*>,feature_t> children() const {}
    constexpr std::expected<std::pair<const attr_t*, const attr_t*>,feature_t> attrs() const {}

    constexpr std::expected<const node_t*,feature_t> parent() const {}
    constexpr std::expected<const unknown_t*,feature_t> prev() const {}
    constexpr std::expected<const unknown_t*,feature_t> next() const {}
};


struct node_t : protected base_t<node_t>{
    private:
    std::string_view _ns;
    std::string_view _name;

    delta_ptr_t _parent;
    delta_ptr_t _next;
    delta_ptr_t _prev;

    delta_ptr_t _children_begin;
    delta_ptr_t _children_end;

    size_t      attrs_count;

    public:

    using base_t::type;
    
    constexpr static inline type_t deftype() {return type_t::NODE;};
    constexpr std::expected<std::string_view,feature_t> ns() const {return _ns;}
    constexpr std::expected<std::string_view,feature_t> name() const {return _name;}
    constexpr std::expected<std::string_view,feature_t> value() const {return std::unexpected(feature_t::NOT_SUPPORTED);}
};

struct attr_t : protected base_t<attr_t>{
    private:
    std::string_view _ns;
    std::string_view _name;
    std::string_view _value;

    public:

    using base_t::type;

    constexpr static inline type_t deftype() {return type_t::ATTR;};
    constexpr std::expected<std::string_view,feature_t> ns() const {return _ns;}
    constexpr std::expected<std::string_view,feature_t> name() const {return _name;}
    constexpr std::expected<std::string_view,feature_t> value() const {return _value;}
};

template<typename T>
struct leaf_t : protected base_t<T>{
    private:
    std::string_view _value;
    delta_ptr_t _parent;
    // delta_ptr_t _next; not needed. Can be statically determined by its size and the children information of the parent.
    delta_ptr_t _prev;

    public:

    using base_t<T>::type;
    leaf_t() = delete;

    constexpr static inline type_t deftype() {return T::deftype();};
    constexpr std::expected<std::string_view,feature_t> ns() const {return std::unexpected(feature_t::NOT_SUPPORTED);}
    constexpr std::expected<std::string_view,feature_t> name() const {return std::unexpected(feature_t::NOT_SUPPORTED);}
    constexpr std::expected<std::string_view,feature_t> value() const {return _value;}
};

struct comment_t : leaf_t<comment_t>{};

struct cdata_t : leaf_t<cdata_t>{};

struct text_t : leaf_t<text_t>{};

struct proc_t : leaf_t<proc_t>{};

struct inject_t : leaf_t<inject_t>{};

static_assert(thing_i<node_t>);
static_assert(thing_i<attr_t>);
static_assert(thing_i<comment_t>);
static_assert(thing_i<cdata_t>);
static_assert(thing_i<text_t>);
static_assert(thing_i<proc_t>);
static_assert(thing_i<inject_t>);

#define DISPATCH(X,Y) \
if (type() == type_t::NODE) return ((const node_t*)this) -> X;\
else if (type() == type_t::ATTR) return ((const attr_t*)this)-> X;\
else if (type() == type_t::TEXT) return ((const text_t*)this)-> X;\
else if (type() == type_t::COMMENT) return ((const comment_t*)this)-> X;\
else if (type() == type_t::PROC) return ((const proc_t*)this)-> X;\
else if (type() == type_t::CDATA) return ((const cdata_t*)this)-> X;\
else if (type() == type_t::INJECT) return ((const inject_t*)this)-> X;\
else{\
    Y;\
}\


struct unknown_t : base_t<unknown_t>{
    constexpr static inline type_t deftype() {return type_t::UNKNOWN;};

    constexpr std::expected<std::string_view,feature_t> ns() const {DISPATCH(ns(),std::terminate());}
    constexpr std::expected<std::string_view,feature_t> name() const {DISPATCH(name(),std::terminate());}
    constexpr std::expected<std::string_view,feature_t> value() const {DISPATCH(value(),std::terminate());}
};


static_assert(thing_i<unknown_t>);


#undef DISPATCH

}


int main(){

    //hello();
    xml::base_t<xml::comment_t> cmt;
    cmt.ns();

    xml::base_t<xml::node_t> hellp;
    auto w = hellp.ns();

    const xml::unknown_t& ww = (const xml::unknown_t&)hellp;
    ww.ns();
    std::print("{}",*hellp.ns());
    //node_t vv;
    //auto q = vv.get<node_t>();
    return 0;
}
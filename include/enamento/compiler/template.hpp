#pragma once

struct scene_forest{
    std::vector<std::string_view> names;
    std::map<std::string, std::shared_ptr<sdf::utils::base_dyn<sdf::default_attrs>>, std::less<void>> nodes;

    so_compiler compiler;

    scene_forest():
    compiler(
        so_compiler::NVIDIA, 
        "/archive/shared/apps/cross-clang/install3/usr/local/bin/clang++",
        "/archive/shared/projects/sdf-new-attempt/include",
        "/archive/shared/projects/sdf-new-attempt/build/src/lib"
    ){

    }

    void set(const std::string& name, const std::shared_ptr<sdf::utils::base_dyn<sdf::default_attrs>>& value){
        auto it = nodes.emplace(name,value);
        if(it.second)names.push_back(it.first->first);
    }

    std::shared_ptr<sdf::utils::base_dyn<sdf::default_attrs>>& get(std::string_view name){
        auto it = nodes.find(name);
        if(it!=nodes.end()){
            return it->second;
        }
        else throw "Not defined";        
    }

    bool compile_to_cpp(std::ostream& out){
        for(auto& name: names){
            auto& node = get(name);
            out<<"static inline auto node_"<<&node<<"=";
            node->to_cpp(out);
            out<<";\n";
        }

        out<<"static inline auto& root = node_"<<&get("$")<<";\n";

        return true;
    }

    bool compile(){
        std::stringbuf buffer;
        std::ostream out(&buffer);

        out<<R"(
            #define GLM_FORCE_INLINE
            #define GLM_FORCE_SWIZZLE
            #include <glm/glm.hpp>
            #include "sdf/sdf.hpp"
            #include <omp.h>

            #define EXPOSE __attribute__((visibility("default"))) extern "C"

            #pragma omp declare target
            EXPOSE int atexit (void (*func)(void)) noexcept{printf("WHATTTTTTT?\nDID I STUTTR?\n");return 0;}
            #pragma omp end declare target

            #pragma omp declare target
            namespace local{
                using namespace glm;
                using namespace sdf::comptime_base;
                struct fake_t{
        )";
        
        if(!compile_to_cpp(out))return false;

        out<<R"(
                }fake;
                sdf::default_attrs _operator(const vec3& pos){return fake.root(pos);}
                float _sample(const vec3& pos){return fake.root.sample(pos);}
            }
            #pragma omp end declare target

            EXPOSE void* addr__operator(int device){
                assert(device<omp_get_num_devices());
                void* tmp=nullptr;
                #pragma omp target device(device) map(tofrom:tmp)
                {
                    tmp=(void*)(&local::_operator);
                }
                return tmp;
            }

            EXPOSE void* addr__sample(int device){
                assert(device<omp_get_num_devices());
                void* tmp=nullptr;
                #pragma omp target device(device) map(tofrom:tmp)
                {
                    tmp=(void*)(&local::_sample);
                }
                return tmp;
            }
        )";

        std::cout<<buffer.view();

        if(!compiler.build(buffer.view())){
            std::cerr << "Library compilation failed.\n";
            return false;
        }

        return true;
    }
};

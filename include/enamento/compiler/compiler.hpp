#pragma once

/**
 * @file compiler.hpp
 * @author karurochari
 * @brief Utils to trigger a compilation task without using libclang which is a mess.
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <ctime>
#include <dlfcn.h>

struct so_compiler{
    namespace fs = std::filesystem;

    public:
        enum platform_t{
            PROCESSOR, NVIDIA, AMD
        };

    private:
        fs::path tmp_src;
        fs::path tmp_so;
        platform_t platform;

        std::string compiler;
        std::string include_path;
        std::string lib_path;

        void *dl_handle=nullptr;

    public:

        so_compiler(platform_t platform, std::string_view compiler = "clang++", std::string_view include_path = {}, std::string_view lib_path = {}):
            platform(platform),compiler(compiler),include_path(include_path),lib_path(lib_path){
 
        }

        ~so_compiler(){
            reset();
        }

        void reset(){
            fs::remove(tmp_src);
            fs::remove(tmp_so);
            if(dl_handle)dlclose(dl_handle);
            dl_handle=nullptr;
        }

        bool build(std::string_view code){
            reset();

            auto time = std::time(0);
            tmp_src = fs::temp_directory_path()/std::format("sdf-{}.cpp",time);
            tmp_so  = fs::temp_directory_path()/std::format("sdf-{}.so",time);

            //Write source file
            {
                std::ofstream ofs(tmp_src);
                if (!ofs) {
                    std::cerr << "Error: Could not open " << tmp_src << " for writing.\n";
                    return false;
                }
                ofs.write(code.data(),code.size());
                ofs.close();
            }

            //Compile
            {
                constexpr const char* platforms[] = {
                    "-fopenmp",
                    "-fopenmp -g -fopenmp-targets=nvptx64 -fopenmp-cuda-mode -fno-use-cxa-atexit",
                    "-fopenmp -g -fopenmp-targets=nvptx64 -fopenmp-cuda-mode"   //TODO: Not implemented
                };

                std::string command = std::format("{} -fvisibility=hidden -O3 -std=c++23 -Wno-return-type-c-linkage -shared -fPIC {} {} -o {} -L{} -I{} -lstdc++ -lvssdf ",compiler,platforms[platform],tmp_src.c_str(),tmp_so.c_str(),lib_path,include_path);
                std::cout << "Compiling library with command: " << command << std::endl;
                int ret = std::system(command.c_str());
                if (ret != 0) {
                    std::cerr << "Compilation failed with code: " << ret << std::endl;
                    return false;
                }
            }

            //Prepare dl handle
            {
                dl_handle = dlopen(tmp_so.c_str(), RTLD_LAZY | RTLD_GLOBAL); //Global needed as I think there is a level of indirection due to openmp. Or something strange like that.
                if (!dl_handle) {
                    std::cerr << "Failed to load library: " << dlerror() << "\n";
                    return false;
                }
                dlerror(); // Reset errors
            }

            return true;
        }
        
        void* handle() const{return dl_handle;}
};
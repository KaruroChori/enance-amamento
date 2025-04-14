#include <iostream>
#include <chrono>
#include <cmath>

#include <omp.h>
#include <mutex>
#include <future>
#include <dlfcn.h>
#include <sstream>

//#define GLM_FORCE_PURE
#define GLM_FORCE_INLINE
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

//This needs to be here BEFORE the first usage of sdf.hpp if shared slots for textures and buffers are desired.
#include "shared-slots.hpp"
#include <sdf/sdf.hpp>

#include <ui/ui.hpp>


const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;


#include <pipeline/basic.hpp>

#include "xml.hpp"
#include "lua-script.hpp"

using namespace glm;


class Editor{
    /*
    selectRoot
    setCursor
        loadXML2Tree
        saveTree2XML
        compileTree         (and load it as jit)
        voxelizeTree
        optimizeTree
        hide
        show
        showOnly
        save
        addOperator
        replaceWith
    setMaterial             set a material entry in the index
    setDriver               setup a driver to check and update at each simulation step.
    setEdtirOption          like border rendering, enable shadows etc.
    */
};

int main(int argc, const char** argv) {
    lua_example();

    const int DEVICE = omp_get_default_device(); //or omp_get_initial_device()

    App::treeview_t treeview;

    std::shared_ptr<sdf::utils::base_dyn<sdf::default_attrs>> fromXML;
    {
        auto doc = pugi::xml_document();
        auto ret = doc.load_file(argc>=2?argv[1]:"./examples/test-0.xml");
        std::cout<<ret;
        try{
            parse_xml<sdf::default_attrs> testxml(doc.first_child());
            fromXML = testxml.sdf_root();
            std::cout<<(fromXML->operator()({0.0,0.0,0.0})).distance<<"\n";

            treeview = testxml.ui_root();
        }catch(...){}
    }

    fromXML->to_cpp((sdf::ostream&)std::cout);

    sdf::tree::builder builder; 
    builder.close(fromXML->to_tree(builder));
    if(!builder.make_shared(2))throw "CannotBuild";

    App app(WINDOW_WIDTH,WINDOW_HEIGHT);

    App::contextual_menu_t menu = {
        {
            {"hello1",[](){std::printf("Hello world!\n");}},
            {"hello2",[](){}, {
                {"hello2.a"},
                {"hello2.b"},
            }},
            {"hello3",[](){}}
        }
    };

    App::details_t details = {};

    auto command_runner = [&](uint64_t ctx,App::commander_action_t action){
        std::print("It was called!\n");
        switch(action){
            case App::commander_action_t::SELECT:
                std::print("Select {}\n",ctx);
                break;
            case App::commander_action_t::HIDE:
                std::print("Hide {}\n",ctx);
                break;
            case App::commander_action_t::SHOW:
                std::print("Show {}\n",ctx);
                break;
            default:
                return false;
        }
        return true;
    };


    sdf::comptime::Interpreted_t<sdf::default_attrs> SDF_MIX_ALL(2);

    sampler::octatree3D::builder sparseA(SDF_MIX_ALL,/*10*/3);
    sparseA.build();
    if(!sparseA.make_shared(3))throw "CannotBuild";
    auto octadata1 = sparseA.stats();

    auto SDF_BASE_W1 = sdf::comptime::OctaSampled3D({3}); 

    static pipeline::material_t materials[10] = {
            {.albedo={.type=pipeline::material_t::albedo_t::COLOR,.color={vec3{1.0,1.0,1.0},1.0}}},
            {.albedo={.type=pipeline::material_t::albedo_t::COLOR,.color={vec3{0.5,0.3,1.0},1.0}}},
            {.albedo={.type=pipeline::material_t::albedo_t::COLOR,.color={vec3{0.2,0.9,0.5},1.0}}},
            {.albedo={.type=pipeline::material_t::albedo_t::COLOR,.color={vec3{0.8,0.8,0.5},1.0}}},
            {.albedo={.type=pipeline::material_t::albedo_t::COLOR,.color={vec3{0.8,0.2,0.2},1.0}}},
            {.albedo={.type=pipeline::material_t::albedo_t::COLOR,.color={vec3{0.2,0.8,0.2},1.0}}}

    };

    //pipeline::demo<decltype(SDF_BASE_W1)> PIPERINE(DEVICE,SDF_BASE_W1,materials,sizeof(materials)/sizeof(pipeline::material_t));
    pipeline::demo<decltype(SDF_MIX_ALL)> PIPERINE(DEVICE,SDF_MIX_ALL,materials,sizeof(materials)/sizeof(pipeline::material_t));

    app.run({
        [&PIPERINE](const App::camera_t& camera, void* buffer){
            PIPERINE.set_camera(camera);
            PIPERINE.render((glm::u8vec4*)buffer);
            return 0;
        },
        [&PIPERINE](const App::camera_t& camera, const glm::vec2& point){
            PIPERINE.set_camera(camera);
            return PIPERINE.raycast(point);
        },
        command_runner,
        menu,
        treeview,
        details
        });
    return 0;
}
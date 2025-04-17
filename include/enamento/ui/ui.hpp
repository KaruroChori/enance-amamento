#pragma once

/**
 * @file ui.hpp
 * @author karurochari
 * @brief Helper to handle the UI of the application to avoid waisting time and space in code.
 * @date 2025-03-09
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#define SDF_HEADLESS

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_keycode.h>

#include <functional>

#include <cstdlib>
#include <print>

#include "solver/projection/base.hpp"

#include "utils/time-series.hpp"

struct App{
    using camera_t = solver::projection::screen_camera_t;

    camera_t camera;

    enum struct commander_action_t {SELECT, HIDE, SHOW};  //TODO: Maybe consider moving it out in case other things will require actions later on

    typedef std::function<int(const camera_t&, void* buffer)> render_t;
    typedef std::function<glm::vec3(const camera_t&, const glm::vec2& point)> caster_t;
    typedef std::function<bool(uint64_t,commander_action_t)> commander_t;

    //typedef std::function<bool(uint64_t cmd)> push_command_t;

    struct contextual_menu_t{
        struct entry_t{
            std::string label;
            //TODO: Add shortcut here as list of keycodes (with implicit modifer needed)
            std::function<void()> op = [](){};
            std::vector<entry_t> children ={};
            bool enabled = true;
        };

        std::vector<entry_t> children;
    };

    
    struct treeview_t{
        struct entry_t{
            std::string label;
            uint64_t ctx;
            std::vector<entry_t> children = {};
            bool enabled = true;
            bool opened = true;
            bool selected = false;
        };

        std::vector<entry_t> children;
    };
    

    struct details_t{
        const char *name;
        sdf::fields_t fields;
        sdf::traits_t traits;
    };

    struct scene_t{
        render_t renderer;
        caster_t raycaster;
        const commander_t& commander;
        contextual_menu_t& ctx_menu;
        treeview_t& tree_view;
        details_t& details;
    };

    App(int width,int height);

    int run(scene_t scene, uint fps=30);

    ~App();

    private:
        void RenderStatsView();
        void RenderCtxMenu(contextual_menu_t&);
        void RenderCtxMenu_inner(const std::vector<contextual_menu_t::entry_t>&);
        void RenderTreeView(treeview_t&, const commander_t& );
        void RenderTreeView_inner(const std::vector<treeview_t::entry_t>&, const commander_t&);
        void RenderDetails(details_t&);
        void RenderStatusBar(const std::vector<std::string>&, const std::vector<std::string>&);

        void resize(int width, int height);

        SDL_Window*     window = nullptr;
        SDL_Renderer*   renderer = nullptr;
        SDL_Texture*    texture = nullptr;
        uint8_t*        buffer = nullptr;
        int             width, height;
        
        bool            ready = false;

        float           fps_target;

        struct settings_t{
            bool show_status = true;
            bool show_camera_data = true;
            bool show_gizmo = true;
            bool show_stats = true;
            bool show_scene_cfg = true;
            bool show_treeview = true;
            bool show_fieldsview = true;
        }settings;

        struct stats_t{
            ScrollingBuffer<2048> fps;
            ScrollingBuffer<2048> fps_avg;
            ScrollingBuffer<2048> resdiv;
            size_t frames = 0;

        }stats;
};

struct sdl_instance{
    sdl_instance(){
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
            std::print(stderr, "SDL_Init Error: {}\n", SDL_GetError());
            throw "SDL_Init failed";
        } 
    }

    ~sdl_instance(){
        SDL_Quit();
    }
};
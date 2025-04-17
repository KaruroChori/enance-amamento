
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <imgui_internal.h>
#include <implot.h>

#include <algorithm>
#include <cmath>

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_scancode.h>
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_INLINE

#include <glm/ext/matrix_transform.hpp>
#include "ui/ui.hpp"
#include "ui/imgui/gizmo.hpp"
#include "ui/imgui/menu-pie.hpp"
#include "ui/imgui/knobs.hpp"
#include "ui/imgui/treeview.hpp"

//#include "ui/panels/terminal.hpp"

#include "ui/panels/details.hpp"

static glm::mat2 rot2D(float angle){
    float s = sin(angle);
    float c = cos(angle);
    return glm::mat2(c,-s,s,c);
}

static float CalcEntriesWidth(const std::vector<std::string>& entries, ImFont* font)
{
    float width = 0.0f;
    for (const auto& entry : entries)
    {
        ImVec2 sz = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, entry.c_str());
        width += sz.x;
        // Add spacing between entries (here 20 pixels, adjust as desired)
        width += 20.0f;
    }
    return width;
}


App::App(int width,int height):width(width),height(height){
    camera.canvas_height=height;
    camera.canvas_width=width;

    window = SDL_CreateWindow("SDL3 Window", width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE );
    if (window == nullptr) {
        std::print(stderr,"SDL_CreateWindow Error: {}\n",SDL_GetError());
        return;
    }

    renderer = SDL_CreateRenderer(window,nullptr);
    if (renderer == nullptr) {
        std::print(stderr,"SDL_CreateRenderer Error: {}\n",SDL_GetError());
        return;
    }

    texture = SDL_CreateTexture(renderer, SDL_PixelFormat::SDL_PIXELFORMAT_RGBA8888, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, width, height);
    if (texture == nullptr) {
        std::print(stderr,"SDL_CreateTexture Error: {}\n",SDL_GetError());
        return;
    }

    buffer = (uint8_t *)malloc(width*height*4);
    
    if (buffer == nullptr) {
        std::print(stderr,"Unable to allocate buffer size for the `app`\n");
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.WantCaptureMouse = true;
    io.WantCaptureKeyboard = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    //ImGuiStyle& style = ImGui::GetStyle();
    //style.AntiAliasedLines = true;
    //style.AntiAliasedFill = true;
    //style.CurveTessellationTol = 1.25f; // Lower values produce more triangles (better curves, but slower)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
    ImPlot::CreateContext();
    ready = true;
}

static bool SmallCheckbox(const char* label, bool* v)
{
    // Save the original style values.
    ImGuiStyle& style = ImGui::GetStyle();
    float backupFramePaddingY = style.FramePadding.y;
    float backupFramePaddingX = style.FramePadding.x;
    float backupItemSpacingX = style.ItemSpacing.x;

    // Use smaller paddings.
    // You can tweak these values until you get your desired look.
    style.FramePadding.y = 2.0f; 
    style.FramePadding.x = 2.0f; 
    style.ItemSpacing.x  = 2.0f;      

    // Optionally, you can also push a smaller font, if one is available.
    // For this example, we'll just stick with the current font and padding modifications.
    bool ret = ImGui::Checkbox(label, v);

    // Restore style values.
    style.FramePadding.y = backupFramePaddingY;
    style.FramePadding.x = backupFramePaddingX;
    style.ItemSpacing.x  = backupItemSpacingX;

    return ret;
}
/*
static void RenderSideTreeView()
{
    // Define the panel width.
    const float panelWidth = 250.0f;

    // Use a child window to render the side panel.
    // Position it on the left side of the main window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, ImGui::GetIO().DisplaySize.y-24));
    
    // Optionally, use flags to remove the title bar, resize, etc.
    ImGui::Begin("Tree View", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
    
    // Begin the tree view
    if (TreeNodeWithToggle("Root  A With long long name"))
    {
        // First branch
        if (TreeNodeWithToggle("Branch A With long long name"))
        {
            ImGui::Text("Leaf A1 ");
            ImGui::Text("Leaf A2");
            // Another nested branch
            if (TreeNodeWithToggle("Branch A nested tt  A With long long name"))
            {
                ImGui::Text("Leaf A Nested 1");
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        // Second branch
        if (TreeNodeWithToggle("Branch B  A With long long name"))
        {
            ImGui::Text("Leaf B1");
            ImGui::Text("Leaf B2");
            ImGui::TreePop();
        }

        // Third branch with checkbox example
        if (TreeNodeWithToggle("Options"))
        {
            static bool option1 = false;
            static bool option2 = true;
            SmallCheckbox("Enable Option 1", &option1);
            SmallCheckbox("Enable Option 2", &option2);
            ImGui::TreePop();
        }

        ImGui::TreePop(); // Close root
    }
    
    ImGui::End(); // End side panel window
}
*/
static void RenderSidePropsView()
{
    struct GlobalSettings
{
    // Camera settings
    float cameraFOV = 60.0f;
    float cameraNear = 0.1f;
    float cameraFar = 1000.0f;

    // Rendering options
    bool enableShadows = true;
    bool useHDR = false;
    int antiAliasing = 2; // e.g. 0=none,1=FXAA,2=TAA

    // Material definition
    float roughness = 0.5f;
    float metallic = 0.0f;
    ImVec4 baseColor = ImVec4(1.0f, 0.5f, 0.25f, 1.0f);
};

struct SelectedSettings
{
    float position[3] = { 0.0f, 0.0f, 0.0f };
    float scale[3] = { 1.0f, 1.0f, 1.0f };
    int materialIndex = 0;
    char guid[64] = "GUID-1234-5678";
    char uuid[64] = "UUID-8765-4321";
};

struct Properties
{
    GlobalSettings global;
    SelectedSettings selected;
};

// Global instance for simplicity.
static Properties g_Properties;

    // Define the panel width.
    const float panelWidth = 350.0f;

    // Use a child window to render the side panel.
    // Position it on the left side of the main window.
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x-panelWidth, 0));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, ImGui::GetIO().DisplaySize.y-24));
    ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);


if (ImGui::BeginChild("PropertiesChild", ImVec2(0, 0), false))
    {
        // Section 1: Global Properties
        if (ImGui::CollapsingHeader("Global Properties", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Use a group and indent to group camera settings.
            ImGui::Text("Camera Settings");
            ImGui::Indent(20);
            ImGui::SliderFloat("FOV", &g_Properties.global.cameraFOV, 30.0f, 120.0f, "%.1f");
            ImGui::InputFloat("Near Plane", &g_Properties.global.cameraNear, 0.1f, 1.0f, "%.2f");
            ImGui::InputFloat("Far Plane", &g_Properties.global.cameraFar, 1.0f, 20.0f, "%.1f");
            ImGui::Unindent(20);
            ImGui::Separator();

            ImGui::Text("Rendering Options");
            ImGui::Indent(20);
            ImGui::Checkbox("Enable Shadows", &g_Properties.global.enableShadows);
            ImGui::Checkbox("Use HDR", &g_Properties.global.useHDR);
            ImGui::SliderInt("Anti-Aliasing", &g_Properties.global.antiAliasing, 0, 4);
            ImGui::Unindent(20);
            ImGui::Separator();

            ImGui::Text("Material Definition");
            ImGui::Indent(20);
            ImGui::SliderFloat("Roughness", &g_Properties.global.roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Metallic", &g_Properties.global.metallic, 0.0f, 1.0f);
            ImGui::ColorEdit4("Base Color", (float*)&g_Properties.global.baseColor);
            ImGui::Unindent(20);
        }

        // Section 2: Selected
        if (ImGui::CollapsingHeader("Selected", ImGuiTreeNodeFlags_DefaultOpen))
        {
            //TODO: Code making use of this information directly should be displaced outside the ui library.
            static uint8_t buffer[128];
            ImGui::Text("Basic fields");
                ui::RenderCfgFields(buffer);
            ImGui::Text("Custom fields");
                //ui::RenderFields(sdf::comptime::Demo_t<sdf::default_attrs>::_fields,sizeof(sdf::comptime::Demo_t<sdf::default_attrs>::_fields)/sizeof(sdf::field_t),buffer);
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

int App::run(scene_t scene, uint fps){
    if(!ready)return 1;
    fps_target=fps;

    bool running = true;
    bool gui_overlay = false;
    SDL_Event event;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    uint64_t last_tick = SDL_GetTicks();
    float last_delta_correction = 1.0;
    glm::vec3 origin={0,0,0};
    uint64_t frames=0;
    float avg_delta = 1000/fps;

	//ImTerm::terminal<terminal_helper_example> terminal_log;
	//terminal_log.set_min_log_level(ImTerm::message::severity::info);

    while (running) {
        if(auto t=scene.renderer(camera,buffer)!=0)return t;

        SDL_UpdateTexture(texture, nullptr, (void*)buffer, width*4);
        SDL_FRect a ={0,0,(float)width,(float)height};
        SDL_FRect b={0,0,(float)width,(float)height};
        SDL_RenderTexture(renderer,texture,&a,&b);

        //SDL_RenderPresent(renderer); // Present the buffer

        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureMouse && !io.WantCaptureKeyboard){
            float mouseX, mouseY;
            uint32_t buttons = SDL_GetMouseState(&mouseX, &mouseY);
            const bool* keys = SDL_GetKeyboardState(NULL);
            /*if(buttons&SDL_BUTTON_MASK(SDL_BUTTON_LEFT)){
                camera.zoom+=(mouseY/height-0.5f)/10.0*last_delta_correction;
            }
            else */
            if(buttons&SDL_BUTTON_MASK(SDL_BUTTON_LEFT) && keys[SDL_SCANCODE_LCTRL]){
                glm::vec2 mouse = (glm::vec2{mouseX,mouseY}-0.5f*glm::vec2{camera.canvas_width,camera.canvas_height})/(float)camera.canvas_height/20.0f*last_delta_correction;
                auto tmp_pos = camera.pos-origin;
                {auto rot = rot2D(-mouse.y); auto to = tmp_pos.yz()*rot;tmp_pos.y=to.x;tmp_pos.z=to.y;}
                {auto rot = rot2D(mouse.x); auto to = tmp_pos.xz()*rot;tmp_pos.x=to.x;tmp_pos.z=to.y;}
                camera.pos=tmp_pos+origin;

                camera.rot+=glm::vec3{-mouse.x,mouse.y,0};
            }

            if(buttons&SDL_BUTTON_MASK(SDL_BUTTON_LEFT) && keys[SDL_SCANCODE_LSHIFT]){
                glm::vec3 mouse = (glm::vec3{mouseX,mouseY,0.0}-0.5f*glm::vec3{camera.canvas_width,camera.canvas_height,0.0})/(float)camera.canvas_height/5.0f*last_delta_correction;
                mouse.x = - mouse.x;
                auto tmp_pos = mouse;
                {auto rot = rot2D(-camera.rot.y); auto to = tmp_pos.yz()*rot;tmp_pos.y=to.x;tmp_pos.z=to.y;}
                {auto rot = rot2D(-camera.rot.x); auto to = tmp_pos.xz()*rot;tmp_pos.x=to.x;tmp_pos.z=to.y;}
                camera.pos+=tmp_pos;
            }
        }

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    resize(event.window.data1, event.window.data2);
                    break;
            }

            ImGui_ImplSDL3_ProcessEvent(&event);
            
            if (!io.WantCaptureMouse && !io.WantCaptureKeyboard){}
            else{continue;}
            
            switch (event.type) {
                case SDL_EVENT_MOUSE_WHEEL:
                    {
                    float mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    glm::vec3 origin_versor = camera.pos-origin;
                    if(glm::length(origin_versor)<0.01)origin_versor={-1,0,0};
                    origin_versor = glm::normalize(origin_versor);
                    camera.pos+=origin_versor*(event.wheel.y)*last_delta_correction;
                    }
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (event.button.button == SDL_BUTTON_MIDDLE) {
                        float mouseX, mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY);
                        origin = scene.raycaster(camera,{mouseX,mouseY});
                    }
                case SDL_EVENT_KEY_DOWN:
                    {
                        if(event.key.scancode==SDL_SCANCODE_HOME || event.key.scancode == SDL_SCANCODE_KP_5){
                            camera.zoom=0;
                            origin = {0,0,0};
                            camera.pos = {0,2,-5};
                            camera.rot = {0,0,0};
                        }
                        else if(event.key.key == SDLK_ESCAPE){
                            gui_overlay=!gui_overlay;
                        }
                        else if(event.key.scancode == SDL_SCANCODE_KP_6){
                            auto tmp_pos = camera.pos-origin;
                            {auto rot = rot2D(-M_PI/2.0f); auto to = tmp_pos.xz()*rot;tmp_pos.x=to.x;tmp_pos.z=to.y;}
                            camera.pos=tmp_pos+origin;
                            camera.rot.x+=M_PI/2.0f;
                        }
                        else if(event.key.scancode == SDL_SCANCODE_KP_4){
                            auto tmp_pos = camera.pos-origin;
                            {auto rot = rot2D(M_PI/2.0f); auto to = tmp_pos.xz()*rot;tmp_pos.x=to.x;tmp_pos.z=to.y;}
                            camera.pos=tmp_pos+origin;
                            camera.rot.x-=M_PI/2.0f;
                        }
                        else if(event.key.scancode == SDL_SCANCODE_KP_2){
                            auto tmp_pos = camera.pos-origin;
                            {auto rot = rot2D(-M_PI/2.0f); auto to = tmp_pos.yz()*rot;tmp_pos.y=to.x;tmp_pos.z=to.y;}
                            camera.pos=tmp_pos+origin;
                            camera.rot.y+=M_PI/2.0f;
                        }
                        else if(event.key.scancode == SDL_SCANCODE_KP_8){
                            auto tmp_pos = camera.pos-origin;
                            {auto rot = rot2D(M_PI/2.0f); auto to = tmp_pos.yz()*rot;tmp_pos.y=to.x;tmp_pos.z=to.y;}
                            camera.pos=tmp_pos+origin;
                            camera.rot.y-=M_PI/2.0f;
                        }
                    }
                    break;
                //TODO: Add joypad
                default:
                    break;
            }
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();

        ImGui::NewFrame();
        {     
            //TODO: Do the math and get the gizmo working for real :/
            if(!gui_overlay && settings.show_gizmo){
                ImOGuizmo::SetRect(40.0f , 40.0f , 100.0f );
                ImOGuizmo::BeginFrame();
                
                glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), camera.rot.z, glm::vec3(1.0f, 0.0f, 0.0f));
                glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), camera.rot.x,  glm::vec3(0.0f, 1.0f, 0.0f));
                glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), -camera.rot.y, glm::vec3(0.0f, 0.0f, 1.0f));

                glm::mat4 rotationMatrix = Rz * Rx * Ry;
                glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -camera.pos);
                glm::mat4 viewMatrix = rotationMatrix * translationMatrix;

                // Example project matrix
                // A 90-degree field of view (FOV)
                // An aspect ratio of 1.0 (square viewport)
                // A near plane at 0.1 and a far plane at 1000.0.
                float projMat[16] = {
                    2.41421f, 0.0f,      0.0f,    0.0f,
                    0.0f,     2.41421f,  0.0f,    0.0f,
                    0.0f,     0.0f,     -1.0002f, -1.0f,
                    0.0f,     0.0f,     -0.20002f, 0.0f
                };
                if(ImOGuizmo::DrawGizmo((float*)&viewMatrix, projMat)){}
            }
            //Coordinates bar
            if(!gui_overlay && settings.show_camera_data){
                // These static arrays hold the values for each 3D vector.
                // Sample data for each of the 3D vectors.
                static float position[3]   = { 1.0f, 2.0f, 3.0f };
                static float rotation[3]   = { 45.0f, 0.0f, -45.0f };
                static float thirdVector[3] = { 0.0f, 5.0f, 10.0f };

                // Define the desired size for the panel.
                ImVec2 panelSize(280, 128);

                // Get the display size (after calling ImGui::NewFrame() in your main loop)
                ImGuiIO& io = ImGui::GetIO();
                ImVec2 displaySize = io.DisplaySize;

                // Calculate top-right position: 
                //   x = display width - panel width - margin, y = margin
                ImVec2 panelPos(displaySize.x - panelSize.x - 0, 0);

                // Set up window flags: remove title bar, resizing, moving, and window decorations.
                ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar 
                                            | ImGuiWindowFlags_NoResize 
                                            | ImGuiWindowFlags_NoMove 
                                            | ImGuiWindowFlags_NoCollapse
                                            | ImGuiWindowFlags_NoScrollbar   // Optional: remove scrollbar if not needed.
                                            | ImGuiWindowFlags_NoScrollWithMouse;   // Optional: remove scrollbar if not needed.

                // Set the window's position and size for this frame.
                ImGui::SetNextWindowPos(panelPos, ImGuiCond_Always);
                ImGui::SetNextWindowSize(panelSize, ImGuiCond_Always);

                // Begin the window. The "Fixed Panel" name is internal and will not be shown (no title bar).
                ImGui::Begin("coordinates", nullptr, windowFlags);

                // Mark the following controls as read-only by disabling them.

                float mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                auto cast = scene.raycaster(camera,{mouseX,mouseY});

                // Display the Position vector.
                ImGui::InputFloat3("Position", (float*)&camera.pos);
                ImGui::InputFloat3("Rotation", (float*)&camera.rot);
                ImGui::InputFloat3("Pivot", (float*)&origin);
                ImGui::InputFloat("Zoom", &camera.zoom);

                ImGui::BeginDisabled(true);

                ImGui::InputFloat3("Ray", (float*)&cast);

                ImGui::EndDisabled();

                ImGui::End();
            }


            std::vector<std::string> leftEntries = {
                std::format("[FPS] {:>6.2f}/{:>6.2f}",1000.0/avg_delta,(float)fps),
                std::format("[SCALE] {:>3.0f}%",1.0/camera.resolution_scale*100),
            };
            std::vector<std::string> rightEntries = {
                "⏰ 12:34",   // a clock icon and time
                "🔔 3"       // a bell icon and number of notifications
            };

            //Status bar
            if(settings.show_status)RenderStatusBar(leftEntries,rightEntries);

            if(settings.show_stats)RenderStatsView();
        }

        if(gui_overlay){
            if(false){
                
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)


                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("focal", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::SliderFloat("lines", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

                if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                if (ImGuiKnobs::Knob("Volume", &f, -6.0f, 6.0f, 0.1f, "%.1fdB", ImGuiKnobs::Variant::Tick)) {
                    // value was changed
                }

                ImGui::End();              

            }

            //RenderSideTreeView();
            if(settings.show_treeview)RenderTreeView(scene.tree_view,scene.commander);

            if(settings.show_fieldsview)RenderSidePropsView();

            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.25f);
            SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            SDL_FRect r={0,0,(float)camera.canvas_width,(float)camera.canvas_height};
            SDL_RenderFillRect(renderer,&r);
            //SDL_RenderClear(renderer);
        }
        else{
            RenderCtxMenu(scene.ctx_menu);
        }      
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        //Logic to handle stable frames and correction for framerate-dependent operations.

        uint64_t old_tick = last_tick;
        last_tick = SDL_GetTicks();

        auto delta = last_tick-old_tick;
        avg_delta=(9.0f*avg_delta+delta)/10.0f;
        
        if(delta<1000.0/(float)fps){
            SDL_Delay( 1000.0/(float)fps - delta );
            last_delta_correction=60.0/fps;
            camera.resolution_scale=glm::clamp(camera.resolution_scale-0.02f,1.0f,10.f);
        }
        else{
            last_delta_correction=delta/(1000/(float)fps)*60.0/fps;
            camera.resolution_scale=glm::clamp(camera.resolution_scale+0.02f,1.0f,10.f);
        }
        if(frames%fps==0){
            SDL_SetWindowTitle(window,std::format("SDF Previewer").c_str());
            //SDL_SetWindowTitle(window,std::format("Hello {:3.2f} fps possible", 1000.0/(float)avg_delta).c_str());
            //std::print("[REPORT] camera.zoom={} camera.pos={},{},{} origin={},{},{} \n",camera.zoom, camera.pos.x, camera.pos.y, camera.pos.z, origin.x, origin.y, origin.z);
        }
        last_tick = SDL_GetTicks();

        stats.fps.AddPoint(frames,1000.0f/(float)delta);
        stats.fps_avg.AddPoint(frames,1000.0f/(float)avg_delta);
        stats.resdiv.AddPoint(frames,camera.resolution_scale);
        stats.frames++;

        frames++;
    }
    return 0;
}

void App::RenderCtxMenu(contextual_menu_t& menu){
    contextual_menu_t::entry_t settings_menu = {"Settings", [](){}, {
        {"UI", [](){}, {
            {"Treeview", [&](){settings.show_treeview=!settings.show_treeview;}},
            {"Details", [&](){settings.show_fieldsview=!settings.show_fieldsview;}},
            {"Stats", [&](){settings.show_stats=!settings.show_stats;}},
            {"Camera", [&](){settings.show_camera_data=!settings.show_camera_data;}},
            {"Gizmo", [&](){settings.show_gizmo=!settings.show_gizmo;}},
            {"Scene", [&](){settings.show_scene_cfg=!settings.show_scene_cfg;}}
        }},
    }};

    if(ImGui::IsMouseClicked(1)){
        ImGui::OpenPopup("PieMenu");
    }
    if(BeginPiePopup("PieMenu", 1)){
        RenderCtxMenu_inner({settings_menu});
        RenderCtxMenu_inner(menu.children);
        EndPiePopup();
    }
}

void App::RenderCtxMenu_inner(const std::vector<contextual_menu_t::entry_t>& menu){
    for(auto& item: menu){
        if(item.children.size()==0){
            if (item.enabled && PieMenuItem(item.label.c_str())) {
                item.op();
            }
        }
        else{
            if (item.enabled && BeginPieMenu(item.label.c_str())) {
                item.op();
                RenderCtxMenu_inner(item.children);
                EndPieMenu();
            }
        }
    }
}


void App::RenderTreeView(treeview_t& menu, const commander_t& commander){
    // Define the panel width.
    const float panelWidth = 250.0f;

    // Use a child window to render the side panel.
    // Position it on the left side of the main window.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, ImGui::GetIO().DisplaySize.y-24));

    //TODO: add check flag if it must be visible.
    ImGui::Begin("Tree View", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

    RenderTreeView_inner(menu.children,commander);

    ImGui::End(); // End side panel window
}

void App::RenderTreeView_inner(const std::vector<treeview_t::entry_t>& menu, const commander_t& commander){
    for(auto& item: menu){
        if (item.enabled ) {
            auto ret = TreeNodeWithToggle(item.label.c_str(),item.opened,item.children.size()==0,item.selected);
            if(ret&1){
                //item.op();
                RenderTreeView_inner(item.children,commander);
                ImGui::TreePop();
            }
            if((ret>>1)==1){commander(item.ctx,commander_action_t::SELECT);}
            else if((ret>>1)==2){/*Probably not needed*/}
            else if((ret>>1)==3){/*Probably not needed*/}
        }
    }
}


void App::RenderStatsView(){

    ImGui::Begin("Stats");


    ImPlot::SetNextAxesToFit();
    if (ImPlot::BeginPlot("Per frame stats")) {
        ImPlot::SetupAxes(nullptr, nullptr, 0,0);
        ImPlot::SetupAxis(ImAxis_Y2);
        ImPlot::SetupAxisLimits(ImAxis_X1,(int)stats.frames-2048, stats.frames, ImGuiCond_Always);
        //ImPlot::SetupAxisLimits(ImAxis_Y1,0,1);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1,0,999);
        ImPlot::SetupAxisLimits(ImAxis_Y2,0,10,ImPlotCond_Always);

        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL,0.5f);
        ImPlot::SetAxis(ImAxis_Y1);
        ImPlot::PlotInfLines("FPS Target",&fps_target,1,ImPlotInfLinesFlags_Horizontal);
        ImPlot::PlotShaded("FPS", &stats.fps.Data[0].x, &stats.fps.Data[0].y, stats.fps.Data.size(), -INFINITY, 0, stats.fps.Offset, 2 * sizeof(float));
        ImPlot::PlotLine("FPS (avg)", &stats.fps_avg.Data[0].x, &stats.fps_avg.Data[0].y, stats.fps_avg.Data.size(), 0, stats.fps_avg.Offset, 2 * sizeof(float));

        ImPlot::SetAxis(ImAxis_Y2);
        ImPlot::PlotLine("Scaling", &stats.resdiv.Data[0].x, &stats.resdiv.Data[0].y, stats.resdiv.Data.size(), 0, stats.resdiv.Offset, 2 * sizeof(float));
ImPlot::EndPlot();
    }
    ImGui::End();
}

void App::RenderStatusBar(const std::vector<std::string>& leftEntries, const std::vector<std::string>& rightEntries){
    ImGuiIO& io = ImGui::GetIO();
    
    // Determine status bar height (you can hardcode a value, or compute it using font size)
    const float statusBarHeight = 25.0f;
    // Get display size from io (or from your windowing system)
    ImVec2 displaySize = io.DisplaySize;

    // Position at the bottom of the screen
    ImGui::SetNextWindowPos(ImVec2(0, displaySize.y - statusBarHeight));
    // Set window size to full display width and determined status bar height
    ImGui::SetNextWindowSize(ImVec2(displaySize.x, statusBarHeight));

    // Set window flags to remove decoration for a status bar feel.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoSavedSettings;

    // Optionally make the window not focusable:
    window_flags |= ImGuiWindowFlags_NoInputs;  // Set this if you want a purely informational status bar

    ImGui::Begin("StatusBar", nullptr, window_flags);

    ImGui::BeginGroup();

    float windowWidth = ImGui::GetWindowWidth();
    ImFont* font = ImGui::GetFont();

    // 1. Draw Left-Aligned Entries
    ImGui::SetCursorPosX(5.0f);  // small left margin
    for (const auto& entry : leftEntries)
    {
        // Here you could call ImGui::Image() instead of ImGui::Text() if you have icons.
        ImGui::TextUnformatted(entry.c_str());
        // Use SameLine to position next entry on the same line with spacing.
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
    }

    ImGui::EndGroup();

    ImGui::SameLine();  
    // Use an invisible dummy widget whose width is the rest of the available space.
    // This forces the right group to shift to the far right.
    ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 0.0f));
    ImGui::SameLine();

    ImGui::BeginGroup();

    // 2. Draw Right-Aligned Entries:
    // Calculate total width of right entries
    float rightEntriesWidth = CalcEntriesWidth(rightEntries, font);
    // Set cursor X to the right margin offset
    float rightStartX = windowWidth - rightEntriesWidth - 5.0f;  // 5 pixel right margin
    ImGui::SetCursorPosX(rightStartX);
    for (const auto& entry : rightEntries)
    {
        ImGui::TextUnformatted(entry.c_str());
        // For all but the last entry, add spacing on the same line
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
    }

    ImGui::EndGroup();

    ImGui::End();
}


App::~App(){
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    free(buffer);
    if(texture!=nullptr)SDL_DestroyTexture(texture);
    if(renderer!=nullptr)SDL_DestroyRenderer(renderer);
    if(window!=nullptr)SDL_DestroyWindow(window);
}

void App::resize(int width, int height){
    this->width=width;
    this->height=height;
    this->camera.canvas_width=width;
    this->camera.canvas_height=height;
    buffer = (uint8_t *)realloc(buffer,width*height*4);
    SDL_DestroyTexture(texture);
    texture = SDL_CreateTexture(renderer, SDL_PixelFormat::SDL_PIXELFORMAT_RGBA8888, SDL_TextureAccess::SDL_TEXTUREACCESS_STREAMING, width, height);
}
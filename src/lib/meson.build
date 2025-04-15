include_dirs = include_directories('../../include')

vssdf = library(
    'vssdf',
    [
        'sdf/sdf.cpp',
        'sdf/codegen.cpp',
        'sdf/serialize.cpp',
    ],
    install: true,
    cpp_args: [openmp_compile_args],
    link_args: [openmp_link_args],
    dependencies: [magic_enum_dep, imgui_dep, pugixml_dep],
    include_directories: include_dirs,
)

vssdf_dep = declare_dependency(
    include_directories: include_dirs,
    link_with: vssdf,
)

vssdf_ui = library(
    'vssdf_ui',
    [
        'ui/ui.cpp',
        'ui/imgui/menu-pie.cpp',
        'ui/imgui/knobs.cpp',
        'ui/imgui/treeview.cpp'
    ],
    install: true,
    dependencies: [imgui_dep, implot_dep, sdl3_dep],
    include_directories: include_dirs,
)

vssdf_ui_dep = declare_dependency(
    include_directories: include_dirs,
    link_with: vssdf_ui,
)

vssdf_scene_import = library(
    'vssdf_scene_import',
    [
        'scene-import/scene.cpp',
    ],
    install: true,
    dependencies: [pugixml_dep, vs_templ_dep, vssdf_dep],
    include_directories: include_dirs,
)

vssdf_scene_import_dep = declare_dependency(
    include_directories: include_dirs,
    link_with: vssdf_scene_import,
)
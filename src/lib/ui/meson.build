vssdf_ui = library(
    'enamento-ui',
    [
        'ui.cpp',
        'imgui/menu-pie.cpp',
        'imgui/knobs.cpp',
        'imgui/treeview.cpp'
    ],
    install: true,
    dependencies: [imgui_dep, implot_dep, sdl3_dep],
    include_directories: [include_dirs, include_directories('include')],
)

install_subdir('../../../include/enamento/ui', install_dir : 'include/enamento', strip_directory: false )

vssdf_ui_dep = declare_dependency(
    include_directories: [include_dirs, include_directories('include')],
    link_with: vssdf_ui,
)
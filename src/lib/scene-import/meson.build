vssdf_scene_import = library(
    'enamento-scene',
    [
        'scene.cpp',
        'lua-bindings.cpp'
    ],
    install: true,
    dependencies: [pugixml_dep, vs_templ_dep, vssdf_dep, lua_dep],
    include_directories: include_dirs,
)

install_subdir('../../../include/enamento/scene-import', install_dir : 'include/enamento', strip_directory: false )

vssdf_scene_import_dep = declare_dependency(
    include_directories: include_dirs,
    link_with: vssdf_scene_import,
)
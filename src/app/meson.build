rr = executable(
    'enamento-demo',
    ['main.cpp','shared-slots.cpp','lua-script.cpp'],
    install: true,
    cpp_args: [openmp_compile_args],
    link_args: [openmp_link_args],
    dependencies: [
        sdl3_dep,
        magic_enum_dep,
        libstub_dep,
        vssdf_dep,
        vssdf_serialize_dep,
        vssdf_ui_dep,
        vssdf_scene_import_dep,
        lua_dep,
        pugixml_dep, #To be removed once the xml loade is done?
        args_dep,
        deps_no_omp
    ],
)

test('basic-run', rr)
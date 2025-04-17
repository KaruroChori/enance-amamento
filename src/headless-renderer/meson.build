headless_renderer = executable(
    'enamento-demo-cli',
    ['main.cpp','shared-slots.cpp'],
    install: true,
    cpp_args: [openmp_compile_args],
    link_args: [openmp_link_args],
    dependencies: [
        magic_enum_dep,
        vssdf_dep,
        vssdf_xml_dep,
        pugixml_dep, #To be removed once the xml loade is done?
        args_dep,
    ],
)
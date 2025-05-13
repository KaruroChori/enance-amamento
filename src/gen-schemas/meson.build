#TODO: add execution of helper.ts if bun is present as pre-step of the main executable.

executable(
    'gen-schemas',
    ['main.cpp'],
    install: false,
    cpp_args: [],
    link_args: [],
    dependencies: [
        vs_xml_dep,
        vssdf_serialize_dep,
        pugixml_dep, #To be removed once the xml loade is done?
        args_dep,
        deps_no_omp
    ],
)


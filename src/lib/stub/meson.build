stub = static_library(
    'enamento-sdf-stub',
    ['stdlib-stub.cpp'],
    install: true,
    cpp_args: [openmp_compile_args],
    link_args: [openmp_link_args],
    dependencies: [magic_enum_dep, imgui_dep, pugixml_dep],
    include_directories: [],
)

libstub_dep = declare_dependency(
    include_directories: [],
    link_with: stub,
)

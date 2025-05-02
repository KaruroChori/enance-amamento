 executable(
    'ui_tester',
    'test-ui.cpp',
    install: false,
    cpp_args: [openmp_compile_args],
    link_args: [openmp_link_args],
    dependencies: [
        vssdf_dep,
        imgui_dep,
        sdl3_dep,
        vssdf_ui_dep,
        deps_no_omp
    ],
    link_with: [],
)

executable(
    'constexpr',
    'test-constexpr.cpp',
    install: false,
    cpp_args: [openmp_compile_args],
    link_args: [openmp_link_args],
    dependencies: [deps_no_omp],
    link_with: [],
)

executable(
    'test-compile',
    'test-compile.cpp',
    install: false,
    cpp_args: [openmp_compile_args],
    link_args: [openmp_link_args],
    dependencies: [deps_no_omp],
    link_with: [],
)

executable(
    'comptime-render',
    'comptime-render.cpp',
    install: false,
    cpp_args: [openmp_compile_args],
    link_args: [openmp_link_args],
    dependencies: [vssdf_dep, deps_no_omp],
    link_with: [],
)


executable(
    'dist-compute',
    'test-dist-compute.cpp',
    install: false,
    cpp_args: [openmp_compile_args],
    link_args: [openmp_link_args],
    dependencies: [vssdf_dep, deps_no_omp],
    link_with: [],
)

mp = executable(
    'test-1',
    'micro/test-1.cpp',
    install: false,
    cpp_args: [openmp_compile_args],
    link_args: [openmp_link_args],
    dependencies: [nanobench_dep, vssdf_dep],

)

benchmark('test-1', mp)
vssdf = library(
    'enamento-sdf',
    [],
    install: true,
    cpp_args: [openmp_compile_args],
    link_args: [openmp_link_args],
    dependencies: [magic_enum_dep, imgui_dep, pugixml_dep, libstub_dep],
    include_directories: include_dirs,
)

#TODO: figure out a way to handle this a bit better.
install_subdir('../../../include/enamento/compiler', install_dir : 'include/enamento', strip_directory: false )
install_subdir('../../../include/enamento/sampler', install_dir : 'include/enamento', strip_directory: false )
install_subdir('../../../include/enamento/solver', install_dir : 'include/enamento', strip_directory: false )
install_subdir('../../../include/enamento/utils', install_dir : 'include/enamento', strip_directory: false )
install_subdir('../../../include/enamento/pipeline', install_dir : 'include/enamento', strip_directory: false )
install_subdir('../../../include/enamento/sdf', install_dir : 'include/enamento', strip_directory: false )

vssdf_dep = declare_dependency(
    include_directories: include_dirs,
    link_with: vssdf,
)

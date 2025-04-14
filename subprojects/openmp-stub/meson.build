project('stub_openmp', 'c',
  version : '1.0',
  default_options : ['warning_level=3', 'c_std=gnu11']
)

# Specify the include directory for header files.
inc_dir = include_directories('include')

# List the source file.
src = files('omp.c')

# Build the static library (no installation, only for internal use)
static_lib = static_library('openmp_stub', src,
  include_directories : inc_dir,
  install : false
)

# Build the shared library (no installation, only for internal use)
shared_lib = shared_library('openmp_stub', src,
  include_directories : inc_dir,
  install : false
)

# Create dependency objects for use in the master project.
# Here we choose to primarily expose the static library dependency,
# but we also create one for the shared library if preferred.
stub_openmp_dep = declare_dependency(
  link_with : static_lib,
  include_directories : inc_dir
)

shared_stub_openmp_dep = declare_dependency(
  link_with : shared_lib,
  include_directories : inc_dir
)

# Export the dependency variables so that the master project can select one:
# For example: dep = subproject('stub_openmp').get_variable('stub_openmp_dep')
#
# If the master project needs the shared variant, it may use:
#    shared_dep = subproject('stub_openmp').get_variable('shared_stub_openmp_dep')
#
# To make it easier to export variables, we simply make them available in our project.
stub_openmp_dep_var = stub_openmp_dep

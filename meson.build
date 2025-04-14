project(
  'enance-amamento',
  'cpp',
  version: '0.1.6',
  default_options: ['warning_level=3', 'cpp_std=c++23'],
)

add_global_arguments(
  ['-Wno-gnu-case-range', '-Wno-zero-length-array', '-Wno-unused-parameter'],
  language: ['cpp', 'c'],
)

openmp_compile_args = [
  '-D SDF_IS_HOST=true',
  '-fopenmp',
  '-g',
  '-fopenmp-targets=' + get_option('offload-targets'),
  '-fopenmp-cuda-mode', '-Xopenmp-target', '-D SDF_IS_HOST=false'
]

#TODO: this override of SDF_IS_HOST is not clean while compiling as it is passed twice for target code.

openmp_link_args = openmp_compile_args

#Dependencies for the main library
glm_dep = dependency('glm', version: '>=1.0.0', modules: ['glm::glm-header-only'])
magic_enum_dep = dependency('magic_enum')
nanobench_dep = dependency('nanobench')

#Extra dependencies for UI and utils
sdl3_dep = dependency('sdl3')
imgui_dep = dependency('imgui')
implot_dep = dependency('implot')
lua_dep = dependency('lua')
pugixml_dep = dependency('pugixml', version: '>=1.14')
vs_templ_dep = dependency('vs_templ', version: '>=0.4.5')
args_dep = dependency('args')

#png_dep = dependency('libpng')
#capnproto_dep = dependency('capnp')

subdir('src/lib')

if get_option('minimal') == false
  subdir('src/app')
  subdir('src/experiments')
endif

if get_option('benchmarks') == true
  subdir('benchmarks')
endif

if get_option('tests') == true
  subdir('tests')
endif
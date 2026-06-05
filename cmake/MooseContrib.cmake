# Contrib libraries that MOOSE itself compiles (as opposed to external deps it discovers):
# pcre (linked into libmoose), hit (the input-file parser, a standalone lib), and gtest
# (for the unit tests). Mirrors framework/moose.mk. Targets are method-suffixed to match the
# co-installed METHOD contract; convenience aliases (Moose::pcre, etc.) are also created.

# pcre: compiled and linked directly into libmoose. Built static+PIC here (the make build
# makes it a separate .la then links it in -- same end result, simpler).
function(moose_add_pcre)
  set(_dir "${CMAKE_SOURCE_DIR}/framework/contrib/pcre")
  file(GLOB_RECURSE _src "${_dir}/*.cc" "${_dir}/*.c")
  moose_method_libname(_name pcre)
  add_library(${_name} STATIC ${_src})
  set_target_properties(${_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_include_directories(${_name} PUBLIC "${_dir}/include")
  target_compile_definitions(${_name} PRIVATE HAVE_CONFIG_H)
  target_link_libraries(${_name} PUBLIC MOOSE::libmesh)
  add_library(Moose::pcre ALIAS ${_name})
endfunction()

# hit: the HIT input-file parser, a standalone shared library MOOSE apps link against.
function(moose_add_hit)
  set(_dir "${CMAKE_SOURCE_DIR}/framework/contrib/hit")
  set(_srcdir "${_dir}/src/hit")
  moose_method_libname(_name hit)
  add_library(${_name} SHARED
    "${_srcdir}/parse.cc" "${_srcdir}/lex.cc" "${_srcdir}/braceexpr.cc")
  target_include_directories(${_name} PUBLIC "${_dir}/include")
  target_link_libraries(${_name} PUBLIC MOOSE::libmesh MOOSE::wasp)
  add_library(Moose::hit ALIAS ${_name})
endfunction()

# gtest: bundled Google Test, used by the unit-test executable. Method-independent in the
# make build; here it lives in the per-method build tree so a plain name is fine.
function(moose_add_gtest)
  set(_dir "${CMAKE_SOURCE_DIR}/framework/contrib/gtest")
  add_library(gtest STATIC "${_dir}/gtest-all.cc")
  set_target_properties(gtest PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_include_directories(gtest PUBLIC "${_dir}")
  add_library(Moose::gtest ALIAS gtest)
endfunction()

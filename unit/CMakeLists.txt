# moose-unit: the framework Google Test executable. Phase 1 of the Make->CMake transition.
# Built non-unity (138 flat source files compile quickly and avoid cross-test unity clashes).

file(GLOB_RECURSE _unit_src CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.C")

moose_method_libname(_unit moose-unit)
add_executable(${_unit} ${_unit_src})

target_include_directories(${_unit} PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/base")

# Matches unit/Makefile: -DMOOSE_UNIT_TEST plus extra warnings.
target_compile_definitions(${_unit} PRIVATE MOOSE_UNIT_TEST)
target_compile_options(${_unit} PRIVATE -Wall -Wextra)

# Moose::moose carries the framework + libMesh/WASP/hit usage requirements; gtest is -isystem.
target_link_libraries(${_unit} PRIVATE Moose::moose Moose::gtest)

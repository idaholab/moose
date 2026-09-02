# moose_export(): generate a build-tree CMake package so external apps can find_package(Moose).
# Writes MooseTargets.cmake (exported Moose::moose, Moose::hit) and MooseConfig.cmake into the
# build dir; point downstream find_package at that dir via PATHS or CMAKE_PREFIX_PATH.
function(moose_export)
  # Exported (un-suffixed) names so downstream links Moose::moose regardless of method; the
  # build tree is method-specific, so one method is consumed at a time.
  set_target_properties(moose-${MOOSE_METHOD} PROPERTIES EXPORT_NAME moose)
  set_target_properties(hit-${MOOSE_METHOD}   PROPERTIES EXPORT_NAME hit)

  export(TARGETS moose-${MOOSE_METHOD} hit-${MOOSE_METHOD}
         NAMESPACE Moose::
         FILE "${CMAKE_BINARY_DIR}/MooseTargets.cmake")

  set(MOOSE_CMAKE_DIR        "${CMAKE_SOURCE_DIR}/cmake")
  set(MOOSE_FRAMEWORK_SCRIPTS "${CMAKE_SOURCE_DIR}/framework/scripts")
  configure_file("${CMAKE_SOURCE_DIR}/cmake/MooseConfig.cmake.in"
                 "${CMAKE_BINARY_DIR}/MooseConfig.cmake" @ONLY)
  message(STATUS "MOOSE package exported: ${CMAKE_BINARY_DIR}/MooseConfig.cmake")
endfunction()

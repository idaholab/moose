# moose_install(): relocatable install of the MOOSE framework as a find_package(Moose) package.
#
#   cmake --install <build> --prefix <prefix>
#
# Installs lib{moose,hit}-<method>, all headers under <prefix>/include/moose, the Moose* cmake
# helpers, and a per-method MooseTargets-<method>.cmake + MooseConfig.cmake. Scoped to the
# framework + hit; exporting modules/combined as COMPONENTS is a further step.
function(moose_install)
  set(_lib   moose-${MOOSE_METHOD})
  set(_hit   hit-${MOOSE_METHOD})

  set_target_properties(${_lib} PROPERTIES EXPORT_NAME moose)
  set_target_properties(${_hit} PROPERTIES EXPORT_NAME hit)

  # Co-installed in lib/moose; @loader_path lets libmoose find libhit beside it. The conda
  # rpaths to libMesh/PETSc/WASP are baked via -Wl,-rpath link flags (not CMake-managed), so
  # they survive install untouched.
  set(_rpath "@loader_path")
  if(NOT APPLE)
    set(_rpath "$ORIGIN")
  endif()
  set_target_properties(${_lib} ${_hit} PROPERTIES INSTALL_RPATH "${_rpath}")

  install(TARGETS ${_lib} ${_hit}
          EXPORT MooseInstallTargets
          LIBRARY DESTINATION lib/moose
          ARCHIVE DESTINATION lib/moose
          RUNTIME DESTINATION lib/moose)

  # Physics modules: install each library to lib/moose (same export set + rpath) and its
  # headers under include/moose/modules/<name>, so downstream can
  # find_package(Moose COMPONENTS <name>). Test libs are not installed (test-only).
  foreach(_m IN LISTS MOOSE_ENABLED_MODULES)
    set(_mlib ${_m}-${MOOSE_METHOD})
    if(TARGET ${_mlib})
      set_target_properties(${_mlib} PROPERTIES EXPORT_NAME ${_m} INSTALL_RPATH "${_rpath}")
      install(TARGETS ${_mlib}
              EXPORT MooseInstallTargets
              LIBRARY DESTINATION lib/moose
              ARCHIVE DESTINATION lib/moose
              RUNTIME DESTINATION lib/moose)
      install(DIRECTORY "${CMAKE_SOURCE_DIR}/modules/${_m}/include/"
              DESTINATION "include/moose/modules/${_m}")
    endif()
  endforeach()

  install(EXPORT MooseInstallTargets
          NAMESPACE Moose::
          FILE MooseTargets-${MOOSE_METHOD}.cmake
          DESTINATION lib/cmake/moose)

  # Headers: framework + each contrib include tree + the generated headers, all under
  # include/moose (the installed config globs every subdir, mirroring the build include set).
  set(_fwk "${CMAKE_SOURCE_DIR}/framework")
  install(DIRECTORY "${_fwk}/include/" DESTINATION include/moose/framework)
  set(_ignore_contrib libtorch mfem neml2 kokkos)
  file(GLOB _contribs LIST_DIRECTORIES true "${_fwk}/contrib/*")
  foreach(_c IN LISTS _contribs)
    if(IS_DIRECTORY "${_c}/include")
      get_filename_component(_cname "${_c}" NAME)
      if(NOT _cname IN_LIST _ignore_contrib)
        install(DIRECTORY "${_c}/include/" DESTINATION "include/moose/contrib/${_cname}")
      endif()
    endif()
  endforeach()
  install(DIRECTORY "${_fwk}/contrib/gtest/" DESTINATION include/moose/contrib/gtest
          FILES_MATCHING PATTERN "*.h")
  # Generated MooseConfig.h / MooseRevision.h at the root of include/moose.
  install(FILES "${MOOSE_CONFIG_HEADER}" "${MOOSE_CONFIG_DIR}/MooseRevision.h"
          DESTINATION include/moose)

  # The cmake helpers a consumer needs (toolchain setup + moose_add_app).
  install(FILES
            "${CMAKE_SOURCE_DIR}/cmake/MooseMethods.cmake"
            "${CMAKE_SOURCE_DIR}/cmake/MooseUtils.cmake"
            "${CMAKE_SOURCE_DIR}/cmake/MooseLibMesh.cmake"
            "${CMAKE_SOURCE_DIR}/cmake/MooseWasp.cmake"
            "${CMAKE_SOURCE_DIR}/cmake/MooseApp.cmake"
          DESTINATION lib/cmake/moose/modules)

  set(MOOSE_FRAMEWORK_SCRIPTS "${CMAKE_SOURCE_DIR}/framework/scripts")
  set(MOOSE_INSTALLED_MODULES "${MOOSE_ENABLED_MODULES}")
  configure_file("${CMAKE_SOURCE_DIR}/cmake/MooseConfigInstalled.cmake.in"
                 "${CMAKE_BINARY_DIR}/install/MooseConfig.cmake" @ONLY)
  install(FILES "${CMAKE_BINARY_DIR}/install/MooseConfig.cmake" DESTINATION lib/cmake/moose)
endfunction()

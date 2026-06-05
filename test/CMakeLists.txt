# The MOOSE test application (moose_test), built via moose_add_app. Phase 2 of the
# Make->CMake transition. Mirrors test/Makefile: BUILD_EXEC=yes, GEN_REVISION=no, the extra
# -Wall -Wextra warnings, and no module dependencies.

moose_add_app(NAME moose_test APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}" BUILD_EXEC)

# test/Makefile adds -Wall -Wextra via ADDITIONAL_CPPFLAGS.
target_compile_options(${MOOSE_APP_LIB_TARGET} PRIVATE -Wall -Wextra)
target_compile_options(${MOOSE_APP_EXEC_TARGET} PRIVATE -Wall -Wextra)

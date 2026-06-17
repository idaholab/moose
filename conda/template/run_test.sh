#!/bin/bash
set -eux

# Activation variables that define the app name
printenv NCRC_APP
printenv NCRC_APP_RAW

# Activation variables for JIT
test "$MOOSE_ADFPARSER_JIT_INCLUDE" == "${PREFIX}/include/moose/ADRealMonolithic.h"
test -f "$MOOSE_ADFPARSER_JIT_INCLUDE"

# Activation executable
BINARY="${NCRC_APP_RAW}-opt"
test "$(which "$BINARY")" == "${PREFIX}/bin/${BINARY}"

# Basic app test with simple diffusion
"$BINARY" -i moose/test/tests/kernels/simple_diffusion/simple_diffusion.i
mpiexec -n 2 "$BINARY" -i moose/test/tests/kernels/simple_diffusion/simple_diffusion.i

# Show app capabilities
"$BINARY" --show-capabilities

# Show inputs that can be copied
"$BINARY" --show-copyable-inputs

# Copy basic tests and do a dry run of runnining them
"$BINARY" --copy-inputs tests
cd "${NCRC_APP_RAW}/tests"
"$BINARY" --run "--dry-run"
cd ../..
rm -rf "$NCRC_APP_RAW"

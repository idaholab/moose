#!/bin/bash
set -eux

# Activation variables for JIT
test "$MOOSE_ADFPARSER_JIT_INCLUDE" == "${PREFIX}/include/moose/ADRealMonolithic.h"
test -f "$MOOSE_ADFPARSER_JIT_INCLUDE"

# Activation executable basic tests
for BINARY in moose moose-opt moose_test-opt combined-opt; do
    test "$(which "$BINARY")" == "${PREFIX}/bin/${BINARY}"
    "$BINARY" -i test/tests/kernels/simple_diffusion/simple_diffusion.i
    mpiexec -n 2 "$BINARY" -i test/tests/kernels/simple_diffusion/simple_diffusion.i
    "$BINARY" --show-capabilities
done

# Copy basic tests and run simple diffusion + a test that needs JIT
moose_test-opt --copy-inputs tests
cd moose_test/tests
moose_test-opt --run '-p 2 --re=kernels/simple_diffusion'
moose_test-opt --run '--re materials/derivative_material_interface.analytic_derivatives/parsed_material'
cd ../..
rm -rf moose_test

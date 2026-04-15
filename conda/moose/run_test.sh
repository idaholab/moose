#!/bin/bash
set -eux

test "$MOOSE_ADFPARSER_JIT_INCLUDE" == "${PREFIX}/include/moose/ADRealMonolithic.h"
test -f "$MOOSE_ADFPARSER_JIT_INCLUDE"

for BINARY in moose moose-opt moose_test-opt combined-opt; do
    test "$(which "$BINARY")" == "${PREFIX}/bin/${BINARY}"
    "$BINARY" --show-capabilities
done

moose_test-opt --copy-inputs tests
cd moose_test/tests
moose_test-opt --run '-p 2 --re=kernels/simple_diffusion'
moose_test-opt --run '-v --re materials/derivative_material_interface.analytic_derivatives/parsed_material'
cd ../..
rm -rf moose_test

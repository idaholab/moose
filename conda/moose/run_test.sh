#!/bin/bash
set -e
# If Intel Mac, forget about it. The environment doesn't seem fit to run
if [[ $(uname) == 'Darwin' ]] && [[ $(uname -m) == 'x86_64' ]]; then
    exit 0
fi
moose_test-opt --copy-inputs tests
cd moose_test/tests
moose_test-opt --run -j 4 --re=kernels/simple_diffusion
exit 0

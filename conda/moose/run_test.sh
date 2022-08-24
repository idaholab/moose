#!/bin/bash
set -e
combined-opt --copy-inputs tests
cd combined/tests
CORES=${MOOSE_JOBS:-2}
combined-opt --run -j $CORES
exit 0

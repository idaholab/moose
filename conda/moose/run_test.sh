#!/bin/bash
set -ex
# A list of module(s)/directories that we do not want to include for testing
NOT_RUNNABLE=(doc module_load combined geochemistry misc navier_stokes tensor_mechanics)

# Support older bash (like Darwin Intel) to build a list of possible modules we want to test
cd ${CONDA_PREFIX}/moose/share/combined
IFS=$'\n' POSSIBLE=(`find . -mindepth 1 -maxdepth 1 -type d -exec basename {} \;`)
cd -

# For my next bash trick... replace spaces for new lines and use `uniq` to remove any matches we
# find in POSSIBLE, and then again for any remainder in NOT_RUNNABLE to clean it up. ACTUALS should
# be what we are actually able to run.
ACTUALS=(`printf '%s\n' ${NOT_RUNNABLE[*]} ${POSSIBLE[*]} ${NOT_RUNNABLE[*]} | sort | uniq -u`)

CORES=${MOOSE_JOBS:-2}
for ACTUAL in ${ACTUALS[@]}; do
    moose --copy-inputs ${ACTUAL}
    cd ${ACTUAL}/tests
    moose --run -j ${CORES}
    cd -
done
exit 0

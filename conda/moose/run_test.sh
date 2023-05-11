#!/bin/bash

# If Intel Mac, forget about it. The environment doesn't seem fit to run
if [[ $(uname) == 'Darwin' ]] && [[ $(uname -m) == 'x86_64' ]]; then
    exit 0
fi

# A list of module(s)/directories that we do not want to include for testing
NOT_RUNNABLE=(doc module_load combined geochemistry misc navier_stokes tensor_mechanics framework external_petsc_solver)

# Support older bash (like Darwin Intel) to build a list of possible modules we want to test
cd ${CONDA_PREFIX}/moose/share/combined
IFS=$'\n' POSSIBLE=(`find . -mindepth 1 -maxdepth 1 -type d -exec basename {} \;`)
_my_temp=`mktemp -d`
cd ${_my_temp}

# For my next bash trick... replace spaces for new lines and use `uniq` to remove any matches we
# find in POSSIBLE, and then again for any remainder in NOT_RUNNABLE to clean it up. ACTUALS should
# be what we are actually able to run.
ACTUALS=(`printf '%s\n' ${NOT_RUNNABLE[*]} ${POSSIBLE[*]} ${NOT_RUNNABLE[*]} | sort | uniq -u`)

CORES=${MOOSE_JOBS:-2}
# TestHarness (Python threads) does not perform well beyond 12 cores
if [ $CORES -ge 12 ]; then CORES=12; fi
EXIT_CODE=0
# A hack for now, to make Darwin work
PLACEHOLDER='tests'
if [[ $(uname) == Darwin ]]; then
    PLACEHOLDER=''
fi
for ACTUAL in ${ACTUALS[@]}; do
    printf "Working on ${ACTUAL}...\n"
    combined-opt --copy-inputs ${ACTUAL}
    cd combined/${ACTUAL}/${PLACEHOLDER}
    combined-opt --run -j ${CORES}
    _last_run=$?
    if [ $_last_run -ge 1 ]; then
        EXIT_CODE=${_last_run}
    fi
    cd -
done

# CLEANUP
cd /tmp
rm -rf ${_my_temp}

exit ${EXIT_CODE}

#!/bin/bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

function check_failure()
{
    # TODO, we will implement some sort of regexp check on the output
    # for the things we see most common (gethostbyname, etc!)
    printf "Checking error for known solutions...
TODO: this function is not yet complete. Basically the idea will be to take your error
and attempt to match it with issues commonly found. You will have to do this yourself
for now... Go to the following address, and see if the error you are receiving matches
anything found there:

https://mooseframework.inl.gov/help/troubleshooting.html
"
}

function test_always_ok()
{
    ./run_tests --${METHOD} -i always_ok -x  | tee fail.log &>/dev/null
    if [ "$?" != "0" ]; then
        if [ -f tests/test_harness/test_harness.always_ok.FAIL.txt ]; then
            cat tests/test_harness/test_harness.always_ok.FAIL.txt
        else
            cat fail.log
        fi
        print_failure_and_exit 'running always_ok'
    fi
    print_green 'OK: Serial Tests\n'
}

function test_parallel()
{
    ./run_tests --${METHOD} -i always_ok -p 2 &>/dev/null
    if [ "$?" != "0" ]; then
        cat tests/test_harness/test_harness.always_ok.FAIL.txt
        print_failure_and_exit 'running parallel test'
    fi
    print_green 'OK: Parallel Tests\n'
}

function test_application()
{
    print_sep
    if [ "${FULL_BUILD}" == 0 ]; then
        printf "Running aggregated tests\n\n"
        enter_moose
        cd test
        set -o pipefail
        test_always_ok
        test_parallel
        print_green '\n\nAll tests passed.\n\n'
        set +o pipefail
    else
        printf "Running all tests...\n\n"
        enter_moose
        cd test
        if [[ ${MOOSE_JOBS:-6} -gt 12 ]]; then
            MOOSE_JOBS=12
        fi
        ./run_tests -j ${MOOSE_JOBS:-6} --${METHOD}
    fi
}

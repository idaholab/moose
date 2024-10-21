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
    if ! ./run_tests "--${METHOD}" -i always_ok -x  | tee fail.log &>/dev/null; then
        if [ -f tests/test_harness/test_harness.always_ok.FAIL.txt ]; then
            cat tests/test_harness/test_harness.always_ok.FAIL.txt
        else
            cat fail.log
        fi
        print_failure_and_exit 'running always_ok'
    fi
    printf '%s: Serial Tests\n' "$(print_green 'OK')"
}

function test_parallel()
{
    if ! ./run_tests "--${METHOD}" -i always_ok -p 2 &>/dev/null; then
        cat tests/test_harness/test_harness.always_ok.FAIL.txt
        print_failure_and_exit 'running parallel test'
    fi
    printf '%s: Parallel Tests\n' "$(print_green 'OK')"
}

function test_application()
{
    print_sep
    export MOOSE_TERM_COLS
    MOOSE_TERM_COLS=$(tput cols)
    if [ "${FULL_BUILD}" == 0 ]; then
        printf "Running aggregated tests\n\n"
        enter_moose
        cd test || return 1
        set -o pipefail
        test_always_ok
        test_parallel
        print_green '\n\nAll tests passed.\n\n'
        set +o pipefail
    else
        printf "Running all tests...\n\n"
        enter_moose
        cd test || return 1
        if [[ ${MOOSE_JOBS:-6} -gt 12 ]]; then
            MOOSE_JOBS=12
        fi
        if ! ./run_tests -j "${MOOSE_JOBS:-6}" "--${METHOD}"; then
            printf '\nFailure(s) detected, running aggregated tests...\n\n'
            if test_always_ok; then
                if ! test_parallel; then
                    printf 'It appears parallel tests may be failing. Please see
our troubleshooting section on our wiki for further help:

    https://mooseframework.inl.gov/help/troubleshooting.html#failingtests

%s: parallel testing failure' "$(print_red 'FAIL')"
                    return 1
                else
                    printf '\nIt appears some tests are failing, but not all. This
can indicate an unsupported system/compiler version, or
other factors we have not yet encountered. Please bring
this to our attention, using our GitHub Discussions
channel, along with a '\`'./diagnostic.sh'\`' report.

If the failure count is small, one can usually proceed
without concern.

%s: partial test failure' "$(print_orange 'WARNING')"
                    return 0
                fi
            else
                printf 'All tests seem to be failing after a successful build.
This may indicate a unique issue with Python, or perhaps
antivirus software, or some other unique environment or
filesystem issue.

%s: All tests failed' "$(print_red 'FAIL')"
                return 1
            fi
        else
            printf '\n\n%s: All tests passed' "$(print_green 'OK')"
        fi
    fi
    return 0
}

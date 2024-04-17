#!/bin/bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

function get_value()
{
    echo $(cat ${SCRIPT_DIR}/../framework/doc/packages_config.yml | grep "^$1: " | awk '{print $2}')
}

function print_failure_and_exit()
{
    if [[ -n "${CANCELED}" ]]; then exit 1; fi
    print_red "ERROR: "
    printf "Printing report.\n"
    env_test
    printf "\n\nThere was an error while $(print_red "$@")\033[0m.

Please report the entirety of the output of diagnostics on
this terminal to either YOUR existing post OR a new post on
https://github.com/idaholab/moose/discussions\n"
    exit_on_failure 1
}

function print_sep()
{
    printf '\n'
    printf '#%.0s' {1..98}
    printf '\n'
}

function print_red()
{
    printf "\033[1;31m$@\033[0m"
}

function print_orange()
{
    printf "\033[38;5;202m$@\033[0m"
}

function print_green()
{
    printf "\033[1;32m$@\033[0m"
}

function run_command()
{
    printf "\033[38;5;242m"
    $@
    exit_code=$?
    printf "\033[0m"
    return $exit_code
}

function create_tmp()
{
    if [ -n "$CTMP_DIR" ]; then echo $CTMP_DIR; return; fi
    if [ `uname` == "Darwin" ]; then
        local TMP_DIR=${TMP_DIR:-`mktemp -d -t MOOSE`}
    else
        local TMP_DIR=${TMP_DIR:-`mktemp -d -t MOOSE.XXXXXXXX`}
    fi
    echo $TMP_DIR
}

function exit_on_failure()
{
    if [ "${NO_FAILURE}" == '1' ]; then
        echo -e "\n--continue-on-fail enabled. Not exiting.\n\n"
        return
    fi
    exit $1
}

function print_help()
{
    args=("-h|--help"\
          "-f|--full-build"\
          "-r|--run-checks"\
          "-c|--conda-channel"\
          "--continue-on-fail"\
          "-v|--verbose")

    args_about=("Print this message and exit."\
                "Build PETSc, libMesh, WASP, MOOSE, and then run entire test suite."\
                "Build MOOSE and then run aggregated tests."\
                "Prioritize MOOSE packages with this channel (default is public)."\
                "Do not stop on failure. If this option works, you have intermittent network issues."\
                "Print additional output from commands being executed.")

    printf "\nSyntax:\n\t./`basename $0`\n\nOptions:\n\n"
    print_args args args_about
    printf "\nInfluencial Environment Variables:\n
\tMETHODS             libMesh build type \"opt dbg devel oprof\" (default: opt)
\tMETHOD              MOOSE build type (default: \"opt\")
\tMOOSE_JOBS          Cores available during \`make -j \$MOOSE_JOBS\`

Proxies/Certificate Authority environmet variables:
(corporate work machines may require that these be set)

\tREQUESTS_CA_BUNDLE=${REQUESTS_CA_BUNDLE:-'not set'}
\tSSL_CERT_FILE=${SSL_CERT_FILE:-'not set'}
\tCURL_CA_BUNDLE=${CURL_CA_BUNDLE:-'not set'}
\thttp_proxy=${HTTP_PROXY:-${http_proxy:-'not set'}}
\thttps_proxy=${HTTPS_PROXY:-${https_proxy:-'not set'}}

Supplying no arguments prints useful environment information.\n\n"
}

function ctl_c()
{
    printf "\033[0m\n\nYou canceled diagnostics. Running clean-up routines.
Please do not ctl-c again, and allow the temp directory to be deleted.\n"
    export CANCELED=true
    exit 1
}

# Check Arguments. If PRISTINE is already set, break out of argument checking
if [ -z "$PRISTINE_ENVIRONMENT" ]; then
    source ${SCRIPT_DIR}/functions/help_system
    while [[ "$#" -gt 0 ]]; do
        case $1 in
            -h|--help)
                print_help; exit 0;;
            -f|--full-build)
                export FULL_BUILD=1; shift ;;
            -r|--run-checks)
                export RUN_CHECKS=1; shift ;;
            -c|--conda-channel)
                if [[ $(echo $2 | grep -c '^http\|^file') -le 0 ]]; then
                    print_red "\ninvalid -c|--conda-channel value\n"
                    print_help; exit 1
                fi
                export CONDA_CHANNEL=$2; shift 2;;
            --continue-on-fail)
                export NO_FAILURE=1; shift ;;
            -v|--verbose)
                export VERBOSITY=1; shift ;;
            *)
                printf "Unknown argument: $1\n"; exit 1;;
        esac
    done
    # default to printing active environment, otherwise do not print active environment
    # (basically I want the default behavior to behave like the previous diagnostic.sh script)
    if [ -n "${FULL_BUILD}" ] || [ -n "${RUN_CHECKS}" ]; then
        NO_ENVIRONMENT=1
    fi
    # FULL_BUILD or RUN_CHECKS is set, but not PRISTINE_ENVIRONMENT
    if [ "${PRISTINE_ENVIRONMENT}" != 0 ] && [ "${NO_ENVIRONMENT}" == 1 ]; then
        # Obtain a temp directory
        export CTMP_DIR=$(create_tmp)
        # Delete temporary directory at any time this script exits
        trap 'printf "\033[0m\n\nDeleting temporary directory\n\n\t${CTMP_DIR}\n"; rm -rf "$CTMP_DIR";' EXIT
        printf "Beginning in a pristine environment\n"
        env -i bash --rcfile <(echo "CLEAN_ENV='True' \
                                PRISTINE_ENVIRONMENT=1 \
                                METHODS=${METHODS:-opt} \
                                METHOD=${METHOD:-opt} \
                                MOOSE_JOBS=${MOOSE_JOBS:-6} \
                                FULL_BUILD=${FULL_BUILD:-0} \
                                RUN_CHECKS=${RUN_CHECKS:-0} \
                                NO_FAILURE=${NO_FAILURE:-0} \
                                VERBOSITY=${VERBOSITY:-0} \
                                CONDA_CHANNEL=${CONDA_CHANNEL:-'https://conda.software.inl.gov/public'} \
                                REQUESTS_CA_BUNDLE=${REQUESTS_CA_BUNDLE:-''} \
                                SSL_CERT_FILE=${SSL_CERT_FILE:-''} \
                                CURL_CA_BUNDLE=${CURL_CA_BUNDLE:-''} \
                                CTMP_DIR=${CTMP_DIR} \
                                NO_ENVIRONMENT=1 \
                                ${SCRIPT_DIR}/${BASH_SOURCE[0]}; exit")
        exit $?
    else
        # Set defaults
        export PRISTINE_ENVIRONMENT=${PRISTINE_ENVIRONMENT:-0}
        export FULL_BUILD=${FULL_BUILD:-0}
        export METHOD=${METHOD:-opt}
        export METHODS=${METHODS:-opt}
        export MOOSE_JOBS=${MOOSE_JOBS:-6}
        export RUN_CHECKS=${RUN_CHECKS:-0}
        export NO_FAILURE=${NO_FAILURE:-0}
        export VERBOSITY=${VERBOSITY:-0}
        export CONDA_CHANNEL=${CONDA_CHANNEL:-'https://conda.software.inl.gov/public'}
        export REQUESTS_CA_BUNDLE=${REQUESTS_CA_BUNDLE:-''}
        export SSL_CERT_FILE=${SSL_CERT_FILE:-''}
        export CURL_CA_BUNDLE=${CURL_CA_BUNDLE:-''}
        export NO_ENVIRONMENT=${NO_ENVIRONMENT:-0}
    fi
fi

# Augment additional pristine environment variables
if [ "${PRISTINE_ENVIRONMENT}" == "1" ]; then
    trap 'ctl_c' INT
    # Further sandbox our environment
    export HOME=${CTMP_DIR}
    # Make our environment more sane
    export PATH=/bin:/usr/bin:/sbin
    export TERM=xterm-256color
    # Make our environment safe
    umask 027
    # It's not enough to set these with --rcfile. They must be exported to be inherited by child processes
    export CTMP_DIR REQUESTS_CA_BUNDLE SSL_CERT_FILE CURL_CA_BUNDLE
    print_sep
    printf "Temporary Directory:\n${CTMP_DIR}\n"
fi

# Make print_environment available in any step
source ${SCRIPT_DIR}/functions/diagnostic_environment.sh

# Do only the one thing if default behavior
if [ "${NO_ENVIRONMENT}" == 0 ]; then print_environment; exit; fi
# Do all the things
printf "
Note: The following steps will be performed in a temporary directory, and will be deleted when
      finished or upon encountering an error.

      The purpose of this tool is to determine if external factors are preventing you from
      building or running MOOSE.

      Errors encountered will usually mean network or hardware related causes (VPN, Network
      Proxies, Corporate SSL Certificates, etc).

      If diagnostics runs to completion without errors then likely the issue will be something in
      your environment as to the cause. If this is case, run `basename $0` again without any
      arguments, and carefully scrutinize the output.\n"
print_sep
source ${SCRIPT_DIR}/functions/diagnostic_conda.sh
source ${SCRIPT_DIR}/functions/diagnostic_application.sh
source ${SCRIPT_DIR}/functions/diagnostic_tests.sh
CONDA_VERSION=$(get_value 'conda_version')

# Get individual packages for now, until moose-mpi becomes available
MPI_PACKAGES="moose-mpi $(get_value 'default_mpi')"
MOOSE_PACKAGES="${MPI_PACKAGES} packaging pyaml jinja2 python"

INSTANCE_EXE='conda'
INSTANCE_SUP='Miniforge3'
install_conda
create_env
build_application
test_application

#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

function get_value()
{
    echo $(cat ${SCRIPT_DIR}/../framework/doc/packages_config.yml | grep "^$1: " | awk '{print $2}')
}

function print_failure_and_exit()
{
    print_environment
    printf "\nThere was an error while $(print_red "$@")\033[0m.

Please report the entirety of the output of diagnostics on
this terminal to either YOUR existing post OR a new post on
https://github.com/idaholab/moose/discussions\n"
    exit 1
}

function print_sep()
{
    printf '\n'
    printf '#%.0s' {1..75}
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

function print_help()
{
    args=("-h|--help"\
          "-f|--full-build"\
          "-r|--run-checks"\
          "-a|--active-environment"\
          "-c|--conda-channel")

    args_about=("Print this message and exit."\
                "Build everything: PETSc, libMesh, WASP, MOOSE."\
                "Run Checks."\
                "Do not use a pristine Conda environment, use current environment instead."\
                "Prioritize MOOSE packages with this channel (default is public).")

    printf "\nSyntax:\n\t./`basename $0`\n\nOptions:\n\n"
    print_args args args_about
    printf "\nInfluencial Environment Variables:\n
\tMETHODS
\tMETHOD
\tMOOSE_JOBS
\tREQUESTS_CA_BUNDLE
\tSSL_CERT_FILE
\tCURL_CA_BUNDLE\n
Supplying no arguments prints useful environment information.\n"
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
            -a|--active-environment)
                export PRISTINE_ENVIRONMENT=0; shift ;;
            -c|--conda-channel)
                export CONDA_CHANNEL=$2; shift 2;;
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
        printf "Beginning in a pristine environment\n"
        env -i bash --rcfile <(echo "CLEAN_ENV='True' \
                                PRISTINE_ENVIRONMENT=1 \
                                METHODS=${METHODS:-opt} \
                                METHOD=${METHOD:-opt} \
                                MOOSE_JOBS=${MOOSE_JOBS:-6} \
                                FULL_BUILD=${FULL_BUILD:-0} \
                                RUN_CHECKS=${RUN_CHECKS:-0} \
                                CONDA_CHANNEL=${CONDA_CHANNEL:-'https://conda.software.inl.gov/public'} \
                                REQUESTS_CA_BUNDLE=${REQUESTS_CA_BUNDLE:-''} \
                                SSL_CERT_FILE=${SSL_CERT_FILE:-''} \
                                CURL_CA_BUNDLE=${CURL_CA_BUNDLE:-''} \
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
        export CONDA_CHANNEL=${CONDA_CHANNEL:-'https://conda.software.inl.gov/public'}
        export REQUESTS_CA_BUNDLE=${REQUESTS_CA_BUNDLE:-''}
        export SSL_CERT_FILE=${SSL_CERT_FILE:-''}
        export CURL_CA_BUNDLE=${CURL_CA_BUNDLE:-''}
        export NO_ENVIRONMENT=${NO_ENVIRONMENT:-0}
    fi
fi

# Augment additional pristine environment variables
if [ "${PRISTINE_ENVIRONMENT}" == "1" ]; then
    # Make our environment more sane
    export PATH=/bin:/usr/bin:/sbin
    export TERM=xterm-256color
    # Make our environment safe
    umask 027
    # Obtain our temp directory
    export CTMP_DIR=$(create_tmp)
    # It's not enough to set these with --rcfile. They must be exported to be inherited by child processes
    export REQUESTS_CA_BUNDLE SSL_CERT_FILE CURL_CA_BUNDLE
    # Delete temporary directory at any time this script exits
    trap 'rm -rf "$CTMP_DIR"' EXIT
    print_sep
    printf "Temporary Directory:\n${CTMP_DIR}\n\n"
fi

# Make print_environment available in any step
source ${SCRIPT_DIR}/functions/diagnostic_environment.sh

# Do only the one thing if default behavior
if [ "${NO_ENVIRONMENT}" == 0 ]; then print_environment; exit; fi
# Do all the things
printf "Note: The following steps are being performed in a temporary directory, and
will be deleted when finished or upon encountering an error. This tool
should only be used as a troubleshooting step to verify that your system is
capable of running MOOSE.

Errors encountered here will likely be key reasons why you might be
experiencing issues.

If no errors are encountered then likely the issue will be something in
your environment as the cause.\n"
source ${SCRIPT_DIR}/functions/diagnostic_conda.sh
source ${SCRIPT_DIR}/functions/diagnostic_application.sh
source ${SCRIPT_DIR}/functions/diagnostic_tests.sh
CONDA_VERSION=$(get_value 'conda_version')

# Get individual packages for now, until moose-mpi becomes available
MPI_PACKAGES="$(get_value 'mpi')=$(get_value $(get_value 'mpi')) $(get_value 'mpi')-mpicc $(get_value 'mpi')-mpicxx $(get_value 'mpi')-mpifort"
SUPPORT_PACKAGES="hdf5=*=mpi_* gfortran cmake make libtool autoconf automake=1.16.5 m4 zfp=0.5.5 bison=3.4 packaging pyaml jinja2 python=3.10"
if [[ `uname` == 'Linux' ]]; then SUPPORT_PACKAGES="libxt-devel-cos7-x86_64=1.1.5 zlib ${SUPPORT_PACKAGES}"; fi
MOOSE_PACKAGES="${MPI_PACKAGES} ${SUPPORT_PACKAGES}"

INSTANCE_EXE='conda'
INSTANCE_SUP='Miniforge3'
print_sep
install_conda
print_sep
create_env
print_sep
build_application
print_sep
test_application

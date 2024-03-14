#!/bin/bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

function conda_url()
{
    local CONDA_URL='https://github.com/conda-forge/miniforge/releases/download'
    echo ${CONDA_URL}/${CONDA_VERSION}/${INSTANCE_SUP}-${CONDA_VERSION}
}

function create_env()
{
    if [ "${PRISTINE_ENVIRONMENT}" == 0 ]; then return; fi
    printf "Creating MOOSE Conda Environment...\n"
    if [ ${FULL_BUILD} == 0 ]; then MOOSE_PACKAGES='moose-dev'; fi
    export CONDARC=$CTMP_DIR/.condarc
    export CONDA_ENVS_PATH=$CTMP_DIR/_env/.envs
    ${INSTANCE_EXE} config --env --set ssl_verify false
    ${INSTANCE_EXE} config --env --set channel_priority strict
    ${INSTANCE_EXE} config --env --add envs_dirs $CTMP_DIR/_env/.envs
    ${INSTANCE_EXE} config --env --add pkgs_dirs $CTMP_DIR/_env/.pkgs
    ${INSTANCE_EXE} config --env --set changeps1 false
    ${INSTANCE_EXE} config --env --set always_yes true
    ${INSTANCE_EXE} config --env --add channels ${CONDA_CHANNEL} &>/dev/null
    ${INSTANCE_EXE} create -p $CTMP_DIR/_env -q -y ${MOOSE_PACKAGES} git git-lfs 1>/dev/null \
    || print_failure_and_exit 'installing Conda environment'
    source activate $CTMP_DIR/_env || print_failure_and_exit 'activating environment'
    printf "The following MOOSE packages were installed:\n\n"
    ${INSTANCE_EXE} list | grep moose
}

function install_conda()
{
    if [ "${PRISTINE_ENVIRONMENT}" == 0 ]; then return; fi
    printf "Installing ${INSTANCE_SUP} @ v${CONDA_VERSION}...\n"
    # Double protect that we will not interfere with existing Conda implementation
    export HOME=${CTMP_DIR}
    if ! `type conda &>/dev/null`; then
        local URL=`conda_url`
        if [ `uname -p` == 'arm' ]; then
            local ARCHFILE="MacOSX-arm64.sh"
        elif [ `uname -p` == 'i386' ]; then
            local ARCHFILE="MacOSX-x86_64.sh"
        else
            local ARCHFILE="Linux-x86_64.sh"
        fi
        local DOWNLOAD_URL="${URL}-${ARCHFILE}"
        curl --insecure -L ${DOWNLOAD_URL} --output \
        ${CTMP_DIR}/install_conda.sh &>/dev/null || exit 1
        bash ${CTMP_DIR}/install_conda.sh \
        -b -p ${CTMP_DIR}/${INSTANCE_EXE} &>/dev/null || print_failure_and_exit 'installing conda'
        export PATH=${CTMP_DIR}/${INSTANCE_EXE}/bin:$PATH
    else
        printf "An existing Conda implementation cannot be avoided (system install perhaps?),
and thus we must exit now.\n\nThis *may* be your reason for not being able to run MOOSE.\n"
        exit 1
    fi
}

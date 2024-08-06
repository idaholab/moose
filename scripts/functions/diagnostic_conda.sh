#!/bin/bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Requires to be sourced by diagnostics.sh
CONDA_VERSION=$(get_value 'conda_version')

INSTANCE_EXE='conda'
INSTANCE_SUP='Miniforge3'

function conda_url()
{
    local CONDA_URL='https://github.com/conda-forge/miniforge/releases/download'
    echo "${CONDA_URL}/${CONDA_VERSION}/${INSTANCE_SUP}-${CONDA_VERSION}"
}

function create_env()
{
    if [ "${PRISTINE_ENVIRONMENT}" == 0 ]; then return; fi
    printf "Creating Conda Environment...\n\n"
    export CONDARC=$CTMP_DIR/.condarc
    export CONDA_ENVS_PATH=$CTMP_DIR/_env/.envs
    ${INSTANCE_EXE} config --env --set channel_priority strict
    ${INSTANCE_EXE} config --env --add envs_dirs "${CTMP_DIR}"/_env/.envs
    ${INSTANCE_EXE} config --env --add pkgs_dirs "${CTMP_DIR}"/_env/.pkgs
    ${INSTANCE_EXE} config --env --set changeps1 false
    ${INSTANCE_EXE} config --env --set always_yes true
    ${INSTANCE_EXE} config --env --add channels "${CONDA_CHANNEL}" &>/dev/null
    if [[ "${VERBOSITY}" == '1' ]]; then
        run_command "${INSTANCE_EXE} create -p $CTMP_DIR/_env -q -y ${MOOSE_PACKAGES} \
        git git-lfs" || print_failure_and_exit 'installing Conda environment'
    else
        # We want word splitting
        # shellcheck disable=SC2086
        ${INSTANCE_EXE} create -p "${CTMP_DIR}"/_env -q -y ${MOOSE_PACKAGES} \
        git git-lfs 1>/dev/null || print_failure_and_exit 'installing Conda environment'
    fi
    # source activate is an available shell function upon PATH'ing conda/bin
    # shellcheck disable=SC1091
    source activate "${CTMP_DIR}"/_env || print_failure_and_exit 'activating environment'
    printf "\nThe following MOOSE packages were installed:\n\n"
    ${INSTANCE_EXE} list | grep 'moose-mpi' | awk '{print $1" "$2" "$3}'
}

function install_conda()
{
    if [[ "${PRISTINE_ENVIRONMENT}" == 0 ]]; then return; fi
    printf 'Installing %s @ v%s...\n' "${INSTANCE_SUP}" "${CONDA_VERSION}"
    if ! type conda &>/dev/null; then
        local URL
        URL=$(conda_url)
        if [[ "$(uname -p)" == 'arm' ]]; then
            local ARCHFILE="MacOSX-arm64.sh"
        elif [[ "$(uname -p)" == 'i386' ]]; then
            local ARCHFILE="MacOSX-x86_64.sh"
        else
            local ARCHFILE="Linux-x86_64.sh"
        fi
        local DOWNLOAD_URL="${URL}-${ARCHFILE}"
        if [[ "${VERBOSITY}" == 1 ]]; then
            run_command "curl -L ${DOWNLOAD_URL} --output ${CTMP_DIR}/install_conda.sh" \
            || exit_on_failure 1
            run_command "bash ${CTMP_DIR}/install_conda.sh -b -p ${CTMP_DIR}/${INSTANCE_EXE}" \
            || print_failure_and_exit 'installing conda'
        else
            curl -L "${DOWNLOAD_URL}" --output "${CTMP_DIR}"/install_conda.sh &>/dev/null \
            || exit_on_failure 1
            bash "${CTMP_DIR}"/install_conda.sh -b -p "${CTMP_DIR}"/"${INSTANCE_EXE}" &>/dev/null \
            || print_failure_and_exit 'installing conda'
        fi
        export PATH=${CTMP_DIR}/${INSTANCE_EXE}/bin:$PATH
    else
        printf "An existing Conda implementation cannot be avoided (system install perhaps?),
and thus we must exit now.\n\nThis *may* be your reason for not being able to run MOOSE.\n"
        exit 1
    fi
}

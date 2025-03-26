#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export REPO=$1
export MOOSE_JOBS=${MOOSE_JOBS:-$2}
export TEMPLATE=template
if [ -n "$3" ]; then
    export TEMPLATE=empty_shell
fi

function create_tmp()
{
    if [ -z "$TMP_DIR" ]; then DO_TRAP="True"; fi
    # Protect running build
    umask 077
    if [ "$(uname)" == 'Darwin' ]; then
        TMP_DIR=${TMP_DIR:-$(mktemp -d -t "${APPLICATION}")}
    else
        TMP_DIR=${TMP_DIR:-$(mktemp -d -t "${APPLICATION}".XXXXXXXX)}
    fi
    if [ -z "${CIVET_HOME}" ] && [ -n "${DO_TRAP}" ]; then
        trap 'rm -rf "$TMP_DIR"' EXIT
    fi
    if [ -d "${TMP_DIR}/${RECIPES}" ]; then
        rm "${TMP_DIR}/${RECIPES}/"*
    fi
    mkdir -p "${TMP_DIR}/${RECIPES}"
}

function create_env()
{
    conda create -p "${TMP_DIR}"/_env -q -y
    # shellcheck disable=SC1091  # 'activate' available after install of miniforge
    source activate "${TMP_DIR}"/_env
    export CONDARC="${TMP_DIR}"/.condarc
    export CONDA_ENVS_PATH="${TMP_DIR}"/_env/.envs
    conda config --env --set ssl_verify false
    conda config --env --set channel_priority strict
    conda config --env --add envs_dirs "${TMP_DIR}"/_env/.envs
    conda config --env --add pkgs_dirs "${TMP_DIR}"/_env/.pkgs
    conda config --env --set changeps1 false
    conda config --env --set always_yes true
    conda config --env --add channels conda-forge
    conda config --env --add channels https://conda.software.inl.gov/public
    conda install conda-build
}

function clean_repo()
{
    printf "Cleaning repo\n"
    if [ "$(git -C "$REPO" status | grep -c -i "^untracked files:")" -ge 1 ] && \
    [ -z "${CIVET_HOME}" ]; then
        read -r -p "Untracked files in ${APPLICATION}. Press Enter to delete files and continue. \
CTRL-C to Cancel. "
    fi
    git -C "${REPO}" clean -xfd &>/dev/null
    git -C "${REPO}" submodule foreach --recursive git clean -xfd &>/dev/null
    git -C "${SCRIPT_DIR}" clean -xfd &> /dev/null
}

function string_replace()
{
    local REPLACE ARG
    printf 'Creating recipes at %s/%s\n' "${TMP_DIR}" "${RECIPES}"
    REPLACE=(APPLICATION FORMATTED_APPLICATION EXECUTABLE REPO BUILD VERSION MOOSE_JOBS MOOSE \
IS_MOOSE TMP_DIR RECIPES SKIP_DOCS PREFIX_PACKAGE_WITH MOOSE_OPTIONS)
    # shellcheck disable=SC2044  # we will never allow spaces in our filesnames
    for sfile in $(find "${SCRIPT_DIR}/${TEMPLATE}" -type l); do
        cat "${sfile}" > "${TMP_DIR}/${RECIPES}/$(basename "${sfile}")"
    done
    # shellcheck disable=SC2044 # we will never allow spaces in our filesnames
    for cfile in $(find "${SCRIPT_DIR}/${TEMPLATE}" -type f); do
        cp "${cfile}" "${TMP_DIR}/${RECIPES}"
        for THIS in "${REPLACE[@]}"; do
            if [ "$(uname)" == 'Darwin' ]; then
                sed -i '' -e "s|<${THIS}>|${!THIS}|g" \
                    "${TMP_DIR}/${RECIPES}/$(basename "${cfile}")"
            else
                sed -i'' -e "s|<${THIS}>|${!THIS}|g" \
                    "${TMP_DIR}/${RECIPES}/$(basename "${cfile}")"
            fi
        done
    done
}

function conda_build()
{
    if [ -z "${CIVET_HOME}" ]; then
        printf "Installing everything necessary to a temporary location (including Conda)\n"
        create_tmp
        printf 'TEMP DIR: %s\n' "${TMP_DIR}"
        create_env
        clean_repo
        string_replace || exit 1
        cd "${TMP_DIR}/${RECIPES}" || exit 1
        mkdir -p "${SCRIPT_DIR}/packages/${APPLICATION}" || exit 1
        conda-build . --output-folder "${SCRIPT_DIR}/packages/${APPLICATION}" || exit 1
        printf 'Built: %s/packages/%s/%s/%s%s-%s-build_%s.conda\n' \
          "${SCRIPT_DIR}" \
          "${APPLICATION}" \
          "${ARCH}" \
          "${PREFIX_PACKAGE_WITH}" \
          "${FORMATTED_APPLICATION}" \
          "${VERSION}" \
          "${BUILD}"
    else
        TMP_DIR="$BUILD_ROOT"
        clean_repo
        create_tmp || exit 1
        string_replace || exit 1
    fi
}

if ! type conda &>/dev/null || ! type activate &>/dev/null; then
    printf 'Conda not installed. Or not properly available (activate not available).\n'
    exit 1
elif [ -z "$2" ]; then
    printf '
Supply possitional arguments:
\t%s /path/to/application jobs

Example:
\t%s /home/user/moose 24
\n' "$0" "$0"
    exit 1
elif ! [ -d "$1" ]; then
    printf '%s not found\n' "${1}"
    exit 1
fi
if [ "$(uname)" == 'Darwin' ]; then
    if [ "$(uname -m)" == 'arm64' ]; then
        ARCH='osx-arm64'
    else
        ARCH='osx-64'
    fi
else
    ARCH='linux-64'
fi
# We are building moose
if [ "$(basename "${REPO}" .git)" == 'moose' ]; then
    export IS_MOOSE='/modules'
    export MOOSE=''
    export EXECUTABLE='combined'
    export PREFIX_PACKAGE_WITH=''
    export MOOSE_SKIP_DOCS='true'
else
    export IS_MOOSE=''
    export MOOSE='/moose'
    export PREFIX_PACKAGE_WITH="${PREFIX_PACKAGE_WITH:-ncrc-}"
fi
SKIP_DOCS='False'
if [ -n "${MOOSE_SKIP_DOCS}" ]; then
    printf "Influential environment variable: MOOSE_SKIP_DOCS detected.\n"
    export SKIP_DOCS='True'
fi
APPLICATION=$(basename "$REPO" .git)
FORMATTED_APPLICATION=$(echo "${APPLICATION}" | tr -dc '[:alnum:]\n\r' | tr '[:upper:]' '[:lower:]')
RECIPES=".recipes/${APPLICATION}"
EXECUTABLE=${EXECUTABLE:-${APPLICATION}}
VERSION=$(git -C "${REPO}" log -1 --date=format:"%Y_%m_%d" --pretty=format:"%ad")
BUILD=0
export APPLICATION FORMATTED_APPLICATION RECIPES EXECUTABLE VERSION BUILD
conda_build
exit 0

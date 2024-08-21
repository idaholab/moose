#!/bin/bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

function compiler_test()
{
    print_sep
    printf "Compiler(s) (CC CXX FC F77 F90):\n\n"
    local compiler_array=(CC CXX FC F77 F90)
    local error_cnt=0
    for compiler in "${compiler_array[@]}"; do
        local no_show=0
        if [[ -n "${!compiler}" ]]; then
            print_bold "which \$${compiler}"
            if ! which "${!compiler}" &>/dev/null; then
                printf ';\t(%s=%s) %s %s' \
                 "${compiler}" \
                 "${!compiler}" \
                 "$(print_bold "${!compiler}")" \
                 "$(print_red "not found")"
                (( error_cnt+=1 ))
                (( no_show+=1 ))
            else
                printf '; %s\n' "$(which "${!compiler}")"
            fi
            if which "${!compiler}" &>/dev/null; then
                print_bold "\$${compiler} --version"
                printf '; %s\n' "$("${!compiler}" --version | head -1)"
            fi
            if [[ $no_show -le 0 ]] && ! ${!compiler} -show &>/dev/null; then
                print_red "FAIL: "
                print_bold "$(basename "${!compiler}")"
                printf ' appears not to be an MPI wrapper'
                (( error_cnt+=1 ))
                (( no_show+=1 ))
            fi
            if [[ ${!compiler} =~ 'mpi' ]] && [[ $no_show -le 0 ]]; then
                if which "${!compiler}" &>/dev/null; then
                    print_bold "\$${compiler} -show"
                    printf '; %s\n' "$(${!compiler} -show)"
                fi
            else
                printf '\n'
            fi
        else
            print_red "${compiler}\tnot set"
            (( error_cnt+=1 ))
        fi
        printf '\n'
    done
    if [[ $error_cnt -ge 1 ]]; then
        print_red "\nFAIL: "
        printf "One or more compiler environment variables not set, or set incorrectly\n"
        return 1
    else
        print_green "OK\n"
    fi
    return 0
}

function env_test()
{
    print_sep
    printf "Influential Environment Variables\n\n"
    reg_exp='^LD\|^DYLD\|^PATH\|^CFLAGS\|^CPP\|^CC\|^CXX\|^FFLAGS\|^FC\|^F90\|^F95\|^F77\|^CONDA'
    reg_exp+='\|^HDF5\|^MOOSE\|^PETSC\|^LIBMESH\|^WASP\|^APPTAINER\|^MODULES\|^PBS\|^SLURM\|^http'
    reg_exp+='\|^HTTPS\|^REQUESTS_CA_BUNDLE\|^SSL_CERT_FILE\|^CURL_CA_BUNDLE\|^FI_PROVIDER'
    reg_not='CONDA_BACKUP'
    env | sort | grep "${reg_exp}" | grep -v "${reg_not}"
}

function python_test()
{
    print_sep
    printf "Python Sanity Checks\n\n"
    my_version="$(/usr/bin/env python3 --version || printf 'NONE')"
    if [[ "${my_version}" != 'NONE' ]]; then
        printf '%s (reporting as: %s) matches\n%s\n\n' \
        "$(print_bold "/usr/bin/env python3 --version");" \
        "${my_version}" \
        "$(print_bold "which python3 python");"
        which_pythons=('python3' 'python')
        local error_cnt=0
        for which_python in "${which_pythons[@]}"; do
            local my_python
            my_python="$(which "${which_python}")"
            # Uncomment to force and fail with different Python
            # my_python="/usr/bin/python3"
            if [[ -n ${my_python} ]] && [[ "$(${my_python} --version)" != "${my_version}" ]]; then
                print_red "FAIL: "
                printf '%s (reporting as %s) != %s\n' \
                "$(print_bold "${my_python} --version")" \
                "$(${my_python} --version))" \
                "${my_version}"
                (( error_cnt+=1 ))
            elif [[ -z "${my_python}" ]] && [[ ${which_python} == 'python' ]]; then
                print_orange "WARNING: "
                print_bold "${which_python};"
                printf ' does not exist. The further back in time we go, the more important this'
                printf ' becomes.\n'
            else
                print_bold "${my_python} --version"
                printf '; == %s\n' "${my_version}"
            fi
        done
    else
        printf "\`python3\` not found\n\n"
        (( error_cnt+=1 ))
    fi
    if [[ $error_cnt -ge 1 ]]; then
        printf "\nThis will likely result in the TestHarness failing. Or WASP/HIT parsing errors.\n"
        return 1
    else
        print_green "\nOK\n"
    fi
    return 0
}

function _python_modules()
{
    print_sep
    printf "Python Modules (TestHarness, run-ability)\n\n"
    fail_modules=(packaging)
    warn_modules=(yaml jinja2)
    local error_cnt=()
    local warn_cnt=()
    for fail_module in "${fail_modules[@]}"; do
        /usr/bin/env python3 -c "import ${fail_module}" 2>/dev/null
        if [[ $? -ge 1 ]]; then
            error_cnt+=("$fail_module ")
        fi
    done
    for warn_module in "${warn_modules[@]}"; do
        /usr/bin/env python3 -c "import ${warn_module}" 2>/dev/null
        if [[ $? -ge 1 ]]; then
            # fix naming convention from package name vs import name
            if [[ ${warn_module} == 'yaml' ]]; then warn_module='pyaml'; fi
            warn_cnt+=("$warn_module ")
        fi
    done
    if [[ ${error_cnt[*]} ]] || [[ ${warn_cnt[*]} ]]; then printf "\n" ;fi
    if [[ ${error_cnt[*]} ]]; then
        print_red 'FAIL:    '
        printf 'missing module(s): %s\n' "$(print_bold "${error_cnt[@]}")"
    fi
    if [[ ${warn_cnt[*]} ]]; then
        print_orange 'WARNING: '
        printf 'missing module(s): %s\n' "$(print_bold "${warn_cnt[@]}")"
    fi
    if [[ ${error_cnt[*]} ]] || [[ ${warn_cnt[*]} ]]; then
        printf '\nEither install the above packages, or perhaps you have yet\n'
        printf 'to activate the moose environment: %s\n\n' "$(print_bold 'conda activate moose')"
        if [[ ${error_cnt[*]} ]]; then
            printf "Missing 'failing' Python modules will prevent you from building MOOSE.\n"
        fi
        if [[ ${warn_cnt[*]} ]]; then
            printf "Missing 'warning' Python modules may cause miscellaneous runtime issues.\n"
        fi
    fi
    if [[ ${error_cnt[*]} ]] || [[ ${warn_cnt[*]} ]]; then
        if [[ ${warn_cnt[*]} ]]; then
            message="$(print_orange 'WARNING')"
        fi
        if [[ ${error_cnt[*]} ]]; then
            message="$(print_red 'FAIL')"
        fi
        printf '\n%s: One or more Python issues present.\n' "${message}"
        return 1
    fi
    print_green "OK\n"
    return 0
}

function conda_test()
{
    # Check library stack compatibility against user's moose repository
    # First run our python_modules test. We need yaml in order to run conda_test.
    _python_modules
    modules_test=$?
    if [[ -z "${CONDA_PREFIX}" ]]; then printf "\n"; return 0; fi
    print_sep
    printf "CONDA MOOSE Package Checks\n\n"
    if [[ $modules_test -ge 1 ]]; then
        print_orange "WARNING: "
        printf "Unable to run Conda tests due to missing Python modules\n\n"
        return 1
    fi
    if [[ -n "${CONDA_EXE}" ]] && [[ -n "${MOOSE_NO_CODESIGN}" ]]; then
        # right to left dependency. `none` is used as a control label
        moose_packages=(dev wasp libmesh petsc none)
        conda_list=$(${CONDA_EXE} list ^moose)

        # iterate over and break on the top-most level we find installed
        for package in "${moose_packages[@]}"; do
            my_version=$(echo -e "${conda_list}" | grep "^moose-${package} " | awk '{print $2}')
            if [ -n "$my_version" ]; then
                # hack naming conventions
                if [[ ${package} == 'dev' ]]; then package='moose-dev'; fi

                local needs_version
                needs_version=()
                while IFS='' read -r line; do needs_version+=("$line"); done < <(./versioner.py \
                 "${package}" --yaml 2>/dev/null | \
                 grep 'install:' | \
                 awk 'BEGIN { FS = "=" }; {print $2}')

                if [[ ${package} != 'moose-dev' ]]; then
                    reminder='Top-level moose-dev package not in use'
                fi
                break
            fi
        done
        if [[ "${package}" == 'none' ]]; then
            printf "\nConda MOOSE dependencies not available (moose-petsc, moose-libmesh,
moose-wasp, etc). User is required to manually build PETSc, libMesh,
and WASP  (see moose/scripts folder)\n\n"
            return 0
        fi
        printf '%s\t%s == %s' "${package}" \
         "$(print_bold "${my_version}")" \
         "$(print_bold "${needs_version[0]}")"

        if [ -n "${reminder}" ]; then printf '\n%s: %s' \
         "$(print_bold 'NOTE')" \
         "${reminder}"
        fi
        if [[ -n "${my_version}" ]] && [[ "${my_version}" != "${needs_version[0]}" ]]; then
            printf "\n%s: %s/repository version mismatch.\n\n" \
             "$(print_red "FAIL")" \
             "${package}"

            printf "Your MOOSE repository requires %s:\t%s
while your Conda environment has %s:\t%s

There are two ways to fix this:

1.  To install the required Conda package version:

\t%s

2.  Or adjust your repository to be in sync with %s Conda package.\n\n" \
 "${package}" \
 "$(print_green "${needs_version[0]}")" \
 "${package}" \
 "$(print_red "${my_version}")" \
 "$(print_bold "conda install ${package}=${needs_version[0]}")" \
 "${package}"

            return 1
        elif [[ -n "${my_version}" ]]; then
            print_green "\n\nOK\n"
        fi
    else
        print_orange "WARNING: "
        printf 'Not using Conda MOOSE packages, or %s not performed\n\n' \
        "$(print_bold 'conda activate moose')"
    fi
    return 0
}

function print_environment()
{
    if [ "${NO_ENVIRONMENT}" == 1 ]; then return; fi
    local ERR_CNT=0
    env_test
    compiler_test || (( ERR_CNT+=1 ))
    python_test || (( ERR_CNT+=1 ))
    conda_test || (( ERR_CNT+=1 ))
    if [[ ${ERR_CNT} -ge 1 ]]; then
      printf '\nchecks %s\n' "$(print_red 'FAILED')"
    else
      printf '\nchecks %s\n' "$(print_green 'PASSED')"
    fi
    exit_on_failure $ERR_CNT
}

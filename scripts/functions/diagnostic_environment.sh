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
                printf '%s: %s does not exist\n' \
                "$(print_orange 'WARNING')" \
                "$(print_bold "${which_python};")"
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
    printf "MOOSE Repository/Conda Version Checks\n\n"
    if [[ $modules_test -ge 1 ]]; then
        print_orange "WARNING: "
        printf "Unable to run Conda tests due to missing Python modules\n\n"
        return 1
    fi
    if [[ -n "${CONDA_EXE}" ]] && [[ -n "${MOOSE_NO_CODESIGN}" ]]; then
        local moose_packages needs_version comparison_version \
        conda_list iter_cnt str_length ver_length
        # right to left dependency. `none` is used as a control label
        moose_packages=(dev libmesh petsc wasp mpi none)
        needs_version=()
        comparison_version=()
        str_length=0
        conda_list=$(${CONDA_EXE} list ^moose)
        # capture required versions
        for package in "${moose_packages[@]}"; do
            if [[ "${package}" == 'dev' ]]; then package='moose-dev'; fi
            while IFS='' read -r line; do needs_version+=("$line"); done < <(./versioner.py \
                "${package}" --yaml 2>/dev/null | \
                grep 'install:' | \
                awk 'BEGIN { FS = "=" }; {print $2}')
         done
        # capture installed versions
        for package in "${moose_packages[@]}"; do
            if [[ $str_length -lt ${#package} ]]; then str_length=${#package}; fi
            my_version=$(echo -e "${conda_list}" | grep "^moose-${package} " | awk '{print $2}')
            if [[ $ver_length -lt ${#my_version} ]]; then ver_length=${#my_version}; fi
            if [ -n "$my_version" ]; then
                comparison_version+=("${package} ${my_version} ${needs_version[$iter_cnt]}")
            else
                comparison_version+=("${package} none ${needs_version[$iter_cnt]}")
            fi
            (( iter_cnt+=1 ))
        done

        # handle comparisons if there are comparisons to be made
        local once mismatch conda_install_packages not_installed
        if [[ -n "${comparison_version[*]}" ]]; then
            for package in "${comparison_version[@]}"; do
                (( once+=1 ))
                if [[ $once -eq 1 ]]; then
                    printf '%*sInstalled%*sRequired\n' "$((str_length+7))" "" "5" ""
                fi
                if [[ $package =~ ([a-zA-Z]+)[[:space:]]([.0-9]+)[[:space:]]([.0-9]+) ]]; then
                    library="${BASH_REMATCH[1]}"
                    installed_version="${BASH_REMATCH[2]}"
                    required_version="${BASH_REMATCH[3]}"
                    # Version mismatch detected
                    if [[ "$installed_version" != "$required_version" ]]; then
                        (( mismatch+=1 ))
                        conda_install_packages+=("moose-${library}=${required_version}")
                        printf '%s %*s' \
                        "$(print_bold "moose-$library")" \
                        "$((str_length-${#library}))" ""
                        printf '%s %*s!= ' \
                        "$(print_red "$installed_version")" \
                        "$((ver_length-${#installed_version}))" ""
                        printf '%s\n' \
                        "$(print_green "$required_version")"
                    # Version match
                    else
                        printf '%s %*s' \
                        "$(print_bold "moose-$library")" \
                        "$((str_length-${#library}))" ""
                        printf '%s %*s== ' \
                        "$(print_green "$installed_version")" \
                        "$((ver_length-${#installed_version}))" ""
                        printf '%s\n' \
                        "$(print_green "$required_version")"
                    fi
                # Package not installed, but list the version required (helps us Discussioners)
                elif [[ $package =~ ([a-zA-Z]+)[[:space:]]none[[:space:]]([.0-9]+) ]]; then
                    library="${BASH_REMATCH[1]}"
                    required_version="${BASH_REMATCH[2]}"
                    not_installed+=("${library}")
                    printf '%s %*s' \
                        "moose-${library}" \
                        "$(( (str_length-${#library}) + ver_length + 4 ))" ""
                    printf '%s\n' "$required_version"
                fi
            done
        else
            printf "\nConda MOOSE dependencies not available (moose-petsc, moose-libmesh,
moose-wasp, etc). User is required to manually build PETSc, libMesh,
and WASP  (see moose/scripts folder)\n\n"
            return 0
        fi
        if [[ -n "${mismatch}" ]]; then
            printf "\n%s: Repository/Conda version mismatch.\n\n" \
             "$(print_red "FAIL")"

            printf "Your MOOSE repository requires different MOOSE Conda packages
than what you have installed.

There are one of two ways to resolve this:

1.  Install the required Conda packages (preferred):

"
            printf '\t%s' "$(print_bold "conda install")"
            for conda_package in "${conda_install_packages[@]}"; do
                if [[ -n "${conda_package}" ]]; then
                    printf ' %s' "$(print_bold "${conda_package}")"
                    # break on anything not moose-wasp (all other packages have dependencies
                    # therefore we can safely end the loop now)
                    if ! [[ $conda_package =~ (moose-wasp) ]]; then
                        printf ' %s' "$(print_bold "$(get_value 'default_mpi')")"
                        break
                    fi
                fi
            done
            printf '\n\t%s\n' \
            "$(print_bold "conda deactivate; conda activate ${CONDA_DEFAULT_ENV}")"
            printf '\n2.  Adjust your repository to be in sync with Conda packages.


Whatever your choice, you will then need to clean any previous
build attempts and start over:\n\n'
            printf '\t%s\n' "$(print_bold "git -C $SCRIPT_DIR/../ clean -Xfd")"
            for conda_package in "${not_installed[@]}"; do
                if [[ -z "${conda_package}" ]] || [[ "${conda_package}" =~ dev ]]; then continue; fi
                if [[ "${conda_package}" =~ petsc ]]; then
                    printf '\t%s\n' "$(print_bold "$SCRIPT_DIR/update_and_rebuild_petsc.sh")"
                fi
                if [[ "${conda_package}" =~ libmesh ]]; then
                    printf '\t%s\n' "$(print_bold "$SCRIPT_DIR/update_and_rebuild_libmesh.sh")"
                fi
                if [[ "${conda_package}" =~ wasp ]]; then
                    printf '\t%s\n' "$(print_bold "$SCRIPT_DIR/update_and_rebuild_wasp.sh")"
                fi
            done
            printf '\t%s\n\t%s\n' \
            "$(print_bold "cd $SCRIPT_DIR/../test")" \
            "$(print_bold "make -j ${MOOSE_JOBS:-6}")"
            return 1
        else
            print_green "\nOK\n"
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

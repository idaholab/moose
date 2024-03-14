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
    printf "Compiler(s) (CC CXX FC F77 F90 F95):\n\n"
    local compiler_array=(CC CXX FC F77 F90 F95)
    local error_cnt=0
    for compiler in ${compiler_array[@]}; do
        if ! [[ "x${!compiler}" == "x" ]]; then
            printf "${compiler}=$(which ${!compiler} 2>/dev/null \
            || printf "${compiler}=${!compiler}\tnot found")\n"
            if [[ $(basename ${!compiler} | grep -c '^mpi') -ge 1 ]]; then
                printf "${compiler} -show:\n$(${!compiler} -show)\n"
            fi
            printf "${compiler} version:\t$(which ${!compiler} &>/dev/null \
            && ${!compiler} --version | head -1)\n\n"
        else
            printf "${compiler}\t\tnot set\n"
            let error_cnt+=1
        fi
    done
    if [[ $error_cnt -ge 1 ]]; then
        print_red "\nFAIL: "
        printf "One or more compiler variables not set\n"
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
    reg_exp+='\|^HDF5\|^MOOSE\|^PETSC\|^LIBMESH\|^WASP\|^APPTAINER\|^MODULES'
    reg_not='CONDA_BACKUP'
    env | sort | grep "${reg_exp}" | grep -v "${reg_not}"
}

function python_test()
{
    print_sep
    printf "Python Sanity Checks\n\n"
    my_version="$(/usr/bin/env python3 --version || printf 'NONE')"
    if [[ "${my_version}" != 'NONE' ]]; then
        printf "Verify \`/usr/bin/env python3 --version\` (reporting as: ${my_version}),
matches versions for: \`which python3 && which python\`\n\n"
        which_pythons=('python3' 'python')
        local error_cnt=0
        for which_python in "${which_pythons[@]}"; do
            local my_python=$(which ${which_python})
            if [[ -n ${my_python} ]] && [[ "$(${my_python} --version)" != "${my_version}" ]]; then
                print_red "FAIL: "
                printf "${my_python} --version (reporting as $(${my_python} --version)) != ${my_version}\n"
                let error_cnt+=1
            elif [[ -z "${my_python}" ]]; then
                print_orange "\nWARNING: "
                printf "\`${which_python}\` does not exist\n"
                printf "This does not mean there will be a failure, but some shebangs in some python
files may still be relying on calling: \`/usr/bin/env ${which_python}\` (Python 2.x era)\n"
            fi
        done
    else
        printf "\`python3\` not found\n\n"
        let error_cnt+=1
    fi
    if [[ $error_cnt -ge 1 ]]; then
        printf "\nThis will likely result in the TestHarness failing. Or WASP/HIT parsing errors.\n"
        return 1
    else
        print_green "OK\n"
    fi
    return 0
}

function _python_modules()
{
    print_sep
    printf "Python Modules (TestHarness, runnability)\n\n"
    check_modules=(packaging yaml pyaml jinja2)
    local error_cnt=0
    for check_module in "${check_modules[@]}"; do
        /usr/bin/env python3 -c "import ${check_module}" 2>/dev/null
        if [[ $? -ge 1 ]]; then
            print_red "FAIL: "
            printf "python module: \`${check_module}\` not available\n"
            let error_cnt+=1
        fi
    done
    if [[ $error_cnt -ge 1 ]]; then
        printf "\nSome Python modules are not available. Numerous miscellaneous issues may
persist without these packages present. Either install these packages, or
perhaps you have yet to \`conda activate moose\`.\n"
        return 1
    else
        print_green "OK\n"
    fi
    return 0
}

function conda_test()
{
    # First run our python_modules test. We need yaml in order to run conda_test.
    _python_modules
    modules_test=$?
    print_sep
    printf "CONDA MOOSE Packages\n\n"
    if [[ $modules_test -ge 1 ]]; then
        print_orange "WARNING: "
        printf "Unable to run Conda tests due to missing Python modules\n\n"
        return 1
    fi
    if [[ -n "${CONDA_PREFIX}" ]] && [[ -n "${MOOSE_NO_CODESIGN}" ]]; then
        # right to left dependency
        moose_packages=(dev wasp libmesh petsc)
        conda_list=`conda list | grep 'moose-'`

        # iterate over and break on the top-most level we find installed
        for package in ${moose_packages[@]}; do
            my_version=$(echo -e "${conda_list}" | grep "^moose-${package} " | awk '{print $2}')
            if [ -n "$my_version" ]; then
                # hack naming conventions
                if [[ ${package} == 'dev' ]]; then prefix='moose-'; fi
                local needs_version=(`./versioner.py "${prefix}${package}" --yaml 2>/dev/null| \
                                      grep 'install:' | \
                                      awk 'BEGIN { FS = "=" }; {print $2" "$3}'`)
                if [[ ${package} != 'dev' ]]; then
                    reminder='moose-dev\t\t  Not present. Careful attention needed\n\t\t\t  when keep everything in sync.'
                fi
                break
            fi
        done
        echo -e "${conda_list}"
        if [ -n "${reminder}" ]; then print_orange "${reminder}\n"; fi
        if [[ -n "${my_version}" ]] && [[ "${my_version}" != "${needs_version}" ]]; then
            print_red "\nFAIL: "
            printf "${prefix}${package}/repository version mismatch\n"
            printf "\nYour MOOSE repository requires moose-${package}:\t$(print_green ${needs_version})
while your Conda environment has moose-${package}:\t$(print_red ${my_version})

There are two ways to fix this:

1.  To install the required Conda package version:

\tconda install moose-${package}=${needs_version}

2.  Or adjust your repository to be in sync with moose-${package} Conda package.\n\n"
            return 1
        elif [[ -n "${my_version}" ]]; then
            print_green "\nOK:"
            printf " Conda packages and MOOSE repo/submodule versions match\n\n"
        else
            printf "\nConda MOOSE packages not applicative.
User is required to manually build PETSc, libMesh, and/or WASP\n\n"
        fi
    else
        print_orange "\nWARNING: "
        printf "Not using Conda MOOSE packages, or \`conda activate moose\` not
performed\n\n"
    fi
    return 0
}

function print_environment()
{
    if [ "${NO_ENVIRONMENT}" == 1 ]; then return; fi
    local ERR_CNT=0
    env_test
    compiler_test || let ERR_CNT+=1
    python_test || let ERR_CNT+=1
    conda_test || let ERR_CNT+=1
    exit $ERR_CNT
}

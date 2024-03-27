#!/bin/bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

function enter_moose()
{
    # TODO: allow a --use-moose-dir argument
    cd $CTMP_DIR/moose || exit_on_failure 1
}

function clone_moose()
{
    # TODO: allow a --use-moose-dir argument
    printf "Creating a clean clone of MOOSE repository\n"
    if [ -z "${retry_cnt}" ]; then
        export retry_cnt=0
    else
        let retry_cnt+=1
    fi
    set -o pipefail
    git clone --depth 1 https://github.com/idaholab/moose ${CTMP_DIR}/moose -b master 2>&1 | tee ${CTMP_DIR}/moose_clone_stdouterr.log
    local exit_code=$?
    set +o pipefail
    if [ ${exit_code} -ge 1 ] && [ $(cat ${CTMP_DIR}/moose_clone_stdouterr.log | grep -c -i 'SSL') -ge 1 ]; then
        if [ -n "${retry_cnt}" ] && [ ${retry_cnt} -ge 2 ]; then
            print_red "\n${retry_cnt} attempt failure.\n"
            exit_on_failure 1
            clone_moose
            return
        elif [ "${GIT_SSL_NO_VERIFY}" == 'true' ]; then
            print_orange "\n${retry_cnt} attempt failure.\n"
            clone_moose
            return
        fi
        print_orange "\nWARNING: "
        printf "SSL issues detected.

This may indicate the root cause of other issues. e.g PETSc contribs may
fail to download properly in later steps, etc

Trying again with protections turned off...\n"
        print_orange "export GIT_SSL_NO_VERIFY=true\n\n"
        export GIT_SSL_NO_VERIFY=true
        clone_moose
        unset GIT_SSL_NO_VERIFY
        return
    elif [ $(cat ${CTMP_DIR}/moose_clone_stdouterr.log | grep -c -i 'SSL') -ge 1 ]; then
        print_orange "\nWARNING: "
        printf "Additional SSL issues detected even after turning GIT SSL
verification off. This indicates a networking issue.

We will continue, but it is very likely we will fail if we attempt to build
PETSc (a full build: -f).\n\n"
        export ALREADY_TRIED_SSL=true
    elif [ ${exit_code} -ge 1 ]; then
        exit_on_failure 1
    fi
    # Print relevant repo data
    git -C ${CTMP_DIR}/moose rev-parse HEAD || exit_on_failure 1
    git -C ${CTMP_DIR}/moose branch || exit_on_failure 1
    git -C ${CTMP_DIR}/moose status || exit_on_failure 1
}

function build_library()
{
    if [ "${FULL_BUILD}" == 0 ]; then return; fi
    print_sep
    local error_cnt=${error_cnt:-0}
    if [ ${error_cnt} -le 0 ]; then printf "Build Step: $1\n"; fi
    enter_moose
    if [ "$1" == 'petsc' ]; then
        # PETSc is special due to all the contribs.
        mkdir -p $CTMP_DIR/downloads
        local petsc_urls=(`scripts/update_and_rebuild_petsc.sh --with-packages-download-dir=$CTMP_DIR/downloads 2>/dev/null | grep "\[" | cut -d, -f 2 | sed -e "s/\]//g"`)
        cd $CTMP_DIR/downloads
        printf "Downloading PETSc contribs...\n"
        for petsc_url in "${petsc_urls[@]}"; do
            clean_url=$(echo ${petsc_url} | sed -e "s/'//g")
            curl --insecure -L -O $clean_url 2>/dev/null
        done
        enter_moose
        printf "Running scripts/update_and_rebuild_${1}.sh using ${MOOSE_JOBS:-6} jobs\n"
        scripts/update_and_rebuild_${1}.sh --skip-submodule-update --with-packages-download-dir=$CTMP_DIR/downloads &> ./${1}_stdouterr.log
    else
        printf "Running scripts/update_and_rebuild_${1}.sh using ${MOOSE_JOBS:-6} jobs, METHODS: ${METHODS}\n"
        scripts/update_and_rebuild_${1}.sh &> ./${1}_stdouterr.log
    fi

    exit_code=$?
    if [ "$exit_code" != '0' ] && [ ${error_cnt} -ge 1 ]; then
        print_failure_and_exit $(tail -20 ./${1}_stdouterr.log)
    elif [ "$exit_code" != '0' ] && [ $(cat ./${1}_stdouterr.log | grep -c -i 'SSL certificate problem') -ge 1 ]; then
        let error_cnt+=1
        print_orange "SSL issues detected, attempting again with SSL protections off\n"
        export GIT_SSL_NO_VERIFY=true
        build_library $1
        # unset so we see it error in each step
        unset GIT_SSL_NO_VERIFY
        return
    elif [ "$exit_code" != '0' ]; then
        tail -20 ${1}_stdouterr.log
        print_failure_and_exit "building $1"
    fi
    printf "Successfully built ${1} ...\n"
}

function build_moose()
{
    printf "Build Step: MOOSE\n"
    enter_moose
    cd test
    METHOD=${METHOD} make -j ${MOOSE_JOBS:-6} &> ./stdouterr.log
    if [ "$?" != '0' ]; then
        tail -20 ./stdouterr.log
        print_failure_and_exit "building MOOSE"
    fi
    printf "Successfully built MOOSE\n"
}

function build_application()
{
    print_sep
    clone_moose
    # Do the dumb necessary things we do in 'moose-mpich' package
    # TODO: remove this when 'moose-mpi' becomes available
    TEMP_CXXFLAGS=${CXXFLAGS//-std=c++[0-9][0-9]}
    ACTIVATION_CXXFLAGS=${TEMP_CXXFLAGS%%-fdebug-prefix-map*}-std=c++17
    export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77 C_INCLUDE_PATH=${CONDA_PREFIX}/include MOOSE_NO_CODESIGN=true MPIHOME=${CONDA_PREFIX} CXXFLAGS="$ACTIVATION_CXXFLAGS" HDF5_DIR=${CONDA_PREFIX} FI_PROVIDER=tcp

    local LIBS=(petsc libmesh wasp)
    for lib in ${LIBS[@]}; do
        build_library ${lib}
    done
    print_sep
    build_moose
}

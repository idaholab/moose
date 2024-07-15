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
    printf "Cloning MOOSE repository\n\n"
    if [ -z "${retry_cnt}" ]; then
        export retry_cnt=0
    else
        let retry_cnt+=1
    fi
    local COMMAND="git clone --depth 1 https://github.com/idaholab/moose ${CTMP_DIR}/moose -b master"
    if [ "${VERBOSITY}" == '1' ]; then
        set -o pipefail
        run_command "${COMMAND}" 2>&1 | tee ${CTMP_DIR}/moose_clone_stdouterr.log
        local exit_code=$?
        set +o pipefail
    else
        ${COMMAND} &> ${CTMP_DIR}/moose_clone_stdouterr.log
        local exit_code=$?
    fi
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
        if [ "${VERBOSITY}" == '0' ]; then
            run_command "tail -15 ${CTMP_DIR}/moose_clone_stdouterr.log"
        fi
        print_orange "\nWARNING: "
        printf "SSL issues detected.

This may indicate the root cause of other issues. e.g MOOSE dependencies may fail to download
properly in later steps.

Possible solutions:

\tYou should contact your network support team and inform them of this issue. In the
\tmeantime you can attempt to disable GIT SSL Verification (not a long-term
\trecommended solution):

\t\texport GIT_SSL_NO_VERIFY=true

Trying again with SSL protections turned off...\n\n"
        export GIT_SSL_NO_VERIFY=true
        clone_moose
        unset GIT_SSL_NO_VERIFY
        return
    elif [ $(cat ${CTMP_DIR}/moose_clone_stdouterr.log | grep -c -i 'SSL') -ge 1 ]; then
        if [ "${VERBOSITY}" == '0' ]; then
            run_command "tail -15 ${CTMP_DIR}/moose_clone_stdouterr.log"
        fi
        print_orange "\nWARNING: "
        printf "Additional SSL issues remain after disabling SSL verification. This indicates
a more serious networking issue which may indicate your inability to build MOOSE dependencies.

Possible solutions:

\tYou should contact your network support team and inform them of this issue.
\tDisable any VPN software for the time being, if possible.
\tYou can attempt to disable SSL Verification in several ways:

\t\texport GIT_SSL_NO_VERIFY=true
\t\tconda config --set ssl_verify false

\tYour network support team should inform you how to properly set the following variables:

\t\tREQUESTS_CA_BUNDLE
\t\tSSL_CERT_FILE
\t\tCURL_CA_BUNDLE
\n"
    elif [ ${exit_code} -ge 1 ]; then
        exit_on_failure 1
    fi
    if [ "${VERBOSITY}" == '1' ]; then
        # Print relevant repo data
        printf "MOOSE Repository information:\n"
        run_command "git -C ${CTMP_DIR}/moose log -1"
        run_command "git -C ${CTMP_DIR}/moose branch"
        run_command "git -C ${CTMP_DIR}/moose status"
    fi
}

function build_library()
{
    if [ "${FULL_BUILD}" == '0' ]; then return; fi
    local error_cnt=${error_cnt:-0}
    if [ ${error_cnt} -le 0 ]; then print_sep; printf "Build Step: $1\n\n"; fi
    enter_moose
    printf "Running scripts/update_and_rebuild_${1}.sh using ${MOOSE_JOBS:-6} jobs, METHODS: ${METHODS}\n"
    if [ "$1" == 'petsc' ]; then
        # PETSc is special due to all the contribs. We need to download each, manualy
        mkdir -p $CTMP_DIR/downloads
        local petsc_urls=(`scripts/update_and_rebuild_petsc.sh --with-packages-download-dir=$CTMP_DIR/downloads 2>/dev/null | grep "\[" | cut -d, -f 2 | sed -e "s/\]//g"`)
        cd $CTMP_DIR/downloads
        printf "Downloading PETSc contribs...\n"
        for petsc_url in "${petsc_urls[@]}"; do
            clean_url=$(echo ${petsc_url} | sed -e "s/'//g")
            if [ "${VERBOSITY}" == '1' ]; then
                printf "\n"
                run_command "curl -L -O $clean_url"
            else
                curl -L -O $clean_url 2>/dev/null
            fi
        done
        enter_moose
        printf "Building PETSc...\n"
        if [ "${VERBOSITY}" == '1' ]; then
            printf "\n"
            set -o pipefail
            run_command "scripts/update_and_rebuild_${1}.sh --skip-submodule-update --with-packages-download-dir=$CTMP_DIR/downloads" 2>&1 | tee ./${1}_stdouterr.log
            exit_code=$?
            set +o pipefail
        else
            scripts/update_and_rebuild_${1}.sh --skip-submodule-update --with-packages-download-dir=$CTMP_DIR/downloads &> ./${1}_stdouterr.log
            exit_code=$?
        fi
    else
        printf "\n"
        if [ "${VERBOSITY}" == '1' ]; then
            set -o pipefail
            run_command "scripts/update_and_rebuild_${1}.sh" 2>&1 | tee ./${1}_stdouterr.log
            exit_code=$?
            set +o pipefail
        else
            scripts/update_and_rebuild_${1}.sh &> ./${1}_stdouterr.log
            exit_code=$?
        fi
    fi

    if [ "$exit_code" != '0' ] && [ ${error_cnt} -ge 1 ]; then
        print_failure_and_exit $(tail -20 ./${1}_stdouterr.log)
    elif [ "$exit_code" != '0' ] && [ $(cat ./${1}_stdouterr.log | grep -c -i 'SSL certificate problem\|No such file or directory') -ge 1 ]; then
        let error_cnt+=1
        if [ "${VERBOSITY}" == '0' ]; then
            run_command "tail -15 ./${1}_stdouterr.log"
        fi
        print_orange "\nWARNING: "
        if [ $(cat ./${1}_stdouterr.log | grep -c -i 'SSL certificate problem') -ge 1 ]; then
            printf "SSL issues detected, attempting again with SSL protections off\n\n"
        elif [ $(cat ./${1}_stdouterr.log | grep -c -i 'No such file or directory') -ge 1 ]; then
            printf "Previous cloning commands failed to download all required packages (most likely due to
SSL issues). Attempting again with SSL protections off.\n\n"
        fi
        export GIT_SSL_NO_VERIFY=true
        build_library $1
        unset GIT_SSL_NO_VERIFY
        return
    elif [ "$exit_code" != '0' ]; then
        if [ "${VERBOSITY}" == '0' ]; then
            run_command "tail -15 ${1}_stdouterr.log"
        fi
        print_failure_and_exit "building $1"
    fi
    printf "Successfully built ${1}\n"
}

function build_moose()
{
    printf "Build Step: MOOSE. Using ${MOOSE_JOBS:-6} cores\n\n"
    enter_moose
    cd test
    if [ "${VERBOSITY}" == '1' ]; then
        set -o pipefail
        export METHOD=${METHOD:-opt}
        run_command "make -j ${MOOSE_JOBS:-6}" 2>&1 | tee ./stdouterr.log
        exit_code=$?
        set +o pipefail
    else
        METHOD=${METHOD:-opt} make -j ${MOOSE_JOBS:-6} &> ./stdouterr.log
        exit_code=$?
    fi
    if [ "$exit_code" != '0' ]; then
        if [ "${VERBOSITY}" == '0' ]; then
            tail -20 ./stdouterr.log
        fi
        print_failure_and_exit "building MOOSE"
    fi
    printf "Successfully built MOOSE\n"
}

function build_application()
{
    print_sep
    clone_moose
    local LIBS=(petsc libmesh wasp)
    for lib in ${LIBS[@]}; do
        build_library ${lib}
    done
    print_sep
    build_moose
}

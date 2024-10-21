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
    if [[ -z "${retry_cnt}" ]]; then
        export retry_cnt=0
    else
        (( retry_cnt+=1 ))
    fi
    local COMMAND exit_code
    COMMAND="git clone --depth 1 https://github.com/idaholab/moose ${CTMP_DIR}/moose -b master"
    if [[ "${VERBOSITY}" == '1' ]]; then
        set -o pipefail
        run_command "${COMMAND}" 2>&1 | tee "${CTMP_DIR}"/moose_clone_stdouterr.log
        exit_code=$?
        set +o pipefail
    else
        ${COMMAND} &> "${CTMP_DIR}"/moose_clone_stdouterr.log
        exit_code=$?
    fi
    if [ ${exit_code} -ge 1 ] \
    && [[ "$(grep -c -i 'SSL' "${CTMP_DIR}"/moose_clone_stdouterr.log)" -ge 1 ]]; then
        if [[ -n "${retry_cnt}" ]] && [[ ${retry_cnt} -ge 2 ]]; then
            print_red "\n${retry_cnt} attempt failure.\n"
            exit_on_failure 1
            clone_moose
            return
        elif [[ "${GIT_SSL_NO_VERIFY}" == 'true' ]]; then
            print_orange "\n${retry_cnt} attempt failure.\n"
            clone_moose
            return
        fi
        if [[ "${VERBOSITY}" == '0' ]]; then
            run_command "tail -15 ${CTMP_DIR}/moose_clone_stdouterr.log"
        fi
        printf '\n%s: SSL issues detected.

This may indicate the root cause of other issues. e.g MOOSE dependencies may fail to download
properly in later steps.

Possible solutions:

\tYou should contact your network support team and inform them of this issue. In the
\tmeantime you can attempt to disable GIT SSL Verification (not a long-term
\trecommended solution):

\t\texport GIT_SSL_NO_VERIFY=true

Trying again with SSL protections turned off...\n\n' "$(print_orange 'WARNING')"
        export GIT_SSL_NO_VERIFY=true
        clone_moose
        unset GIT_SSL_NO_VERIFY
        return
    elif [[ "$(grep -c -i 'SSL' "${CTMP_DIR}"/moose_clone_stdouterr.log)" -ge 1 ]]; then
        if [[ "${VERBOSITY}" == '0' ]]; then
            run_command "tail -15 ${CTMP_DIR}/moose_clone_stdouterr.log"
        fi
printf '\n%s: Additional SSL issues remain after disabling SSL
verification. This indicates a more serious networking
issue which may indicate your inability to build MOOSE
dependencies.

Possible solutions:

\tYou should contact your network support team
\tand inform them of this issue. Disable any VPN
\tsoftware for the time being, if possible. You
\tcan attempt to disable SSL Verification in
\tseveral ways:

\t\t%s
\t\t%s

\tYour network support team should inform you how
\tto properly set the following Conda influential
\tenvironment variables:

\t\t%s
\t\t%s
\t\t%s
\n' \
"$(print_orange 'WARNING')" \
"$(print_bold 'export GIT_SSL_NO_VERIFY=true')" \
"$(print_bold 'conda config --set ssl_verify false')" \
"$(print_bold 'REQUESTS_CA_BUNDLE')" \
"$(print_bold 'SSL_CERT_FILE')" \
"$(print_bold 'CURL_CA_BUNDLE')"

    elif [[ "${exit_code}" -ge 1 ]]; then
        exit_on_failure 1
    fi
    if [[ "${VERBOSITY}" == '1' ]]; then
        # Print relevant repo data
        printf "MOOSE Repository information:\n"
        run_command "git -C ${CTMP_DIR}/moose log -1"
        run_command "git -C ${CTMP_DIR}/moose branch"
        run_command "git -C ${CTMP_DIR}/moose status"
    fi
}

function build_library()
{
    if [[ "${FULL_BUILD}" == '0' ]]; then return; fi
    local error_cnt
    local library
    library="${1}"
    error_cnt=${error_cnt:-0}
    if [[ "${error_cnt}" -le 0 ]]; then print_sep; printf 'Build Step: %s\n\n' "${library}"; fi
    if [[ "${error_cnt}" -ge 3 ]]; then printf 'Too many failures to continue.'; exit 1; fi
    enter_moose
    printf "Running scripts/update_and_rebuild_%s.sh using %s jobs, METHODS: %s\n" \
    "${library}" \
    "${MOOSE_JOBS:-6}" \
    "${METHODS:-opt}"
    # PETSc is special due to all the contribs. We need to download each, manualy
    if [[ "${library}" == 'petsc' ]]; then
        local petsc_urls
        petsc_urls=()
        mkdir -p "$CTMP_DIR"/downloads
        scripts/update_and_rebuild_petsc.sh --with-packages-download-dir="$CTMP_DIR"/downloads \
          2>/dev/null > ./url_file
        while IFS='' read -r line; do petsc_urls+=("$line"); done \
          < <(grep "\[" ./url_file | cut -d, -f 2 | sed -e "s/\]//g")
        cd "${CTMP_DIR}"/downloads || exit 1
        printf "Downloading PETSc contribs...\n"
        for petsc_url in "${petsc_urls[@]}"; do
            clean_url=${petsc_url//\'/}
            if [[ "${VERBOSITY}" == '1' ]]; then
                printf "\n"
                run_command "curl -L -O ${clean_url}"
            else
                curl -L -O "${clean_url}" 2>/dev/null
            fi
        done
        enter_moose
        printf "Building PETSc...\n"
        if [[ "${VERBOSITY}" == '1' ]]; then
            printf "\n"
            set -o pipefail
            run_command scripts/update_and_rebuild_"${library}".sh \
             --skip-submodule-update \
             --with-packages-download-dir="$CTMP_DIR"/downloads 2>&1 \
             | tee ./"${library}"_stdouterr.log
            exit_code=$?
            set +o pipefail
        else
            scripts/update_and_rebuild_"${library}".sh \
             --skip-submodule-update \
             --with-packages-download-dir="$CTMP_DIR"/downloads &> ./"${library}"_stdouterr.log
            exit_code=$?
        fi
    else
        printf "\n"
        if [[ "${VERBOSITY}" == '1' ]]; then
            set -o pipefail
            run_command "scripts/update_and_rebuild_${library}.sh" 2>&1 \
             | tee ./"${library}"_stdouterr.log
            exit_code=$?
            set +o pipefail
        else
            scripts/update_and_rebuild_"${library}".sh &> ./"${library}"_stdouterr.log
            exit_code=$?
        fi
    fi

    if [[ "$exit_code" != '0' ]] && [[ "${error_cnt}" -ge 1 ]]; then
        print_failure_and_exit "$(tail -20 ./"${library}"_stdouterr.log)"
    elif [[ "$exit_code" != '0' ]]; then
        (( error_cnt+=1 ))
        ssl_errors=$(grep -c -i 'SSL certificate problem' ./"${library}"_stdouterr.log)
        lib_missing=$(grep -c -i 'No such file or directory' ./"${library}"_stdouterr.log)
        if [[ "${VERBOSITY}" == '0' ]]; then
            run_command "tail -15 ./${library}_stdouterr.log"
        fi
        print_orange "\nWARNING: "
        if [[ "${ssl_errors}" -ge 1 ]]; then
            printf "SSL issues detected, attempting again with SSL protections off\n\n"
        elif [[ "${lib_missing}" -ge 1 ]]; then
            printf "Previous cloning commands failed to download all required packages (most likely due to
SSL issues). Attempting again with SSL protections off.\n\n"
        fi
        export GIT_SSL_NO_VERIFY=true
        build_library "${library}"
        unset GIT_SSL_NO_VERIFY
        return
    elif [[ "$exit_code" != '0' ]]; then
        if [[ "${VERBOSITY}" == '0' ]]; then
            run_command "tail -15 ${library}_stdouterr.log"
        fi
        print_failure_and_exit "building ${library}"
    fi
    printf 'Successfully built %s\n' "${library}"
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
    unset PETSC_DIR LIBMESH_DIR WASP_DIR
    print_sep
    clone_moose
    local LIBS=(petsc libmesh wasp)
    for lib in "${LIBS[@]}"; do
        build_library "${lib}"
    done
    print_sep
    build_moose
}

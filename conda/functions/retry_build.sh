#!/bin/bash
function can_retry(){
    local TRY_AGAIN_REASON=('Library not loaded: @rpath/')
    for reason in "${TRY_AGAIN_REASON[@]}"; do
        if [[ $(grep -c "${reason}" "${1}") -ge 1 ]]; then
            return 1
        fi
    done
    return 0
}
function no_exit_failure(){
  set +e
  (
    set -o pipefail
    do_build 2>&1 | tee -a "${SRC_DIR}"/output.log
  )
}
function retry_build(){
    while true; do
        if no_exit_failure; then
            set -e
            break
        elif can_retry "${SRC_DIR}"/output.log; then
            tail -600 "${SRC_DIR}"/output.log && exit 1
        elif [[ ${try_count} -gt 1 ]]; then
            tail -50 "${SRC_DIR}"/output.log
            (( try_count++ )) || true
            printf "Exhausted retry attempts: %s\n" "${try_count}"
            exit 1
        fi
        (( try_count++ )) || true
        tail -100 output.log
        printf "\n\nRoutine error caught, trying again...\n"
        true > "${SRC_DIR}"/output.log
    done
}

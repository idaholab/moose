#!/bin/sh
# allow user overrides if ${HOME}/.bashrc_appatiner` exists
# NOTE: `source ${HOME}/.bashrc_appatiner` is not enough to support user profile bash functions
if test -f "${HOME}/.bashrc_apptainer" ; then
    user_override="${HOME}/.bashrc_apptainer"
    action="${0##*/}"
    case "${action}" in
    shell)
        if [ "${SINGULARITY_SHELL:-}" = "/bin/bash" ]; then
            set -- --noprofile --rcfile "${user_override}"
        elif test -z "${SINGULARITY_SHELL:-}"; then
            export SINGULARITY_SHELL=/bin/bash
            set -- --noprofile --rcfile "${user_override}"
        fi
        ;;
    exec)
        export BASH_ENV="${user_override}"
        set -- /bin/bash --noprofile --rcfile "${user_override}" -c "$*"
        ;;
    run)
        set -- /bin/bash --noprofile --rcfile "${user_override}"
    esac
fi

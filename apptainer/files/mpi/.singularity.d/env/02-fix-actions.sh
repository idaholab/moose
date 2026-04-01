#!/bin/sh
# Get the action from the singularity command; this will be either
# shell, exec, or run
ACTION="${0##*/}"

# "apptainer shell"
if [ "$ACTION" = "shell" ]; then
    # Default to bash if not specified
    if test -z "${SINGULARITY_SHELL:-}"; then
        export SINGULARITY_SHELL="/bin/bash"
    fi

    # Using bash; apply our custom bashrc if available
    if [ "$SINGULARITY_SHELL" = "/bin/bash" ]; then
        SHELL_BASHRC="/.singularity.d/moose_bashrc"
        if test -f "$SHELL_BASHRC"; then
            set -- --rcfile "$SHELL_BASHRC" "$@"
        fi
    fi
# "apptainer exec ..."; execute as a command within bash,
# as long as the command isn't a shell binary
elif [ "$ACTION" = "exec" ]; then
    if [ "$1" != "/bin/bash" ] && [ "$1" != "bash" ] && [ "$1" != "/bin/sh" ] && [ "$1" != "sh" ]; then
        set -- /bin/bash --noprofile --norc -c 'exec "$@"' /bin/bash "$@"
    fi
fi

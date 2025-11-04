action="${0##*/}"
if [[ "${action}" == 'exec' ]]; then
    set -- /bin/bash --noprofile -c "$*"
fi

#!/bin/bash

# If a user supplies an argument, that directory will be used as the start point for finding files that contain whitespace, otherwise the MOOSE directory
# will be used (one up from the scripts directory where this script is located)
REPO_DIR=${1:-"$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../"}

hookfile="$REPO_DIR/.git/hooks/pre-commit"

if [[ -f $hookfile ]]; then
    echo "'$hookfile' already exists - aborting" 1>&2
    exit 1
fi

echo '
#!/bin/bash
CHECK_AUTOFORMAT=0 ./scripts/moose-format.sh
fail=$?
./scripts/autofmt.sh
if [[ "$?" != "0" ]]; then
    fail=1
fi

exit $fail
' > $hookfile

chmod a+x $hookfile


#!/bin/bash

REPO_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../"

hookfile="$REPO_DIR/.git/hooks/pre-commit"

if [[ -f $hookfile ]]; then
    echo "'$hookfile' already exists - aborting" 1>&2
    exit 1
fi

echo '
#!/bin/bash
./scripts/autofmt.sh
exit $?
' > $hookfile

chmod a+x $hookfile


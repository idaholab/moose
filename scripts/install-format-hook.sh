#!/bin/bash

REPO_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../"
hookfile="$REPO_DIR/.git/hooks/pre-commit"

if [[ -f $hookfile ]]; then
    echo "'$hookfile' already exists - aborting" 1>&2
    exit 1
fi

echo '#!/bin/bash
patch=$(git clang-format --diff)
if [[ "$patch" == "no modified files to format" ]]; then
    echo "passed"
else
    echo "formatting fixes required" >&2
    echo ""
    echo "$patch"
    exit 1
fi
' > $hookfile

chmod a+x $hookfile


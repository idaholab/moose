#!/bin/bash

REPO_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/../"
hookfile="$REPO_DIR/.git/hooks/pre-commit"

if [[ -f $hookfile ]]; then
    echo "'$hookfile' already exists - aborting" 1>&2
    exit 1
fi

echo '#!/bin/bash
patch=$(git clang-format --diff -- $(git diff --staged --name-only -- src include tests unit))
if [[ "$patch" =~ "no modified files to format" || "$patch" =~ "clang-format did not modify any files" ]]; then
    echo "" > /dev/null
else
    echo ""
    echo "Your code is not properly formatted." >&2
    echo "Run 'git clang-format' to resolve the following issues:" >&2
    echo ""
    echo "$patch"
    exit 1
fi
' > $hookfile

chmod a+x $hookfile


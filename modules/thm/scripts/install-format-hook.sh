#!/bin/bash

# This script installs a clang-format git pre-commit hook, which formats all staged files
# with clang-format prior to a commit. This script can be run from any directory. If the
# hook file already exists, an error message will be printed.

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
hookfile="$(cd $script_dir && git rev-parse --show-toplevel)/.git/hooks/pre-commit"

# ensure that hook file does not exist yet
if [[ -f $hookfile ]]; then
    echo "ERROR: The pre-commit hook '$hookfile' already exists." 1>&2
    echo "       If you would like to replace it, then remove it and run this script again." 1>&2
    exit 1
fi

# create the hook file and make it executable
echo '#!/bin/bash

# Pre-commit hook to run clang-format on all staged c++ files.

# paths to all c++ source and header files
cpp_file_paths="include src test/include test/src unit/include unit/src"

# get names of all staged files that are added, copied, modified, or renamed; exclude deleted files
changed_cpp_files=`git diff --staged --diff-filter=ACMR --name-only -- $cpp_file_paths`

git clang-format -- $changed_cpp_files
git add $changed_cpp_files
' > $hookfile

chmod a+x $hookfile

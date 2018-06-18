#!/bin/bash

# This script installs a clang-format git pre-commit hook for MOOSE, which
# formats all staged files with clang-format prior to a commit, either for a
# stand-alone MOOSE git repository, or for MOOSE as a submodule of another git
# repository. This script can be run from any directory. If the hook file
# already exists, an error message will be printed.

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# determine the appropriate hook file
function moose_is_git_submodule()
{
    cd $script_dir && git rev-parse --show-superproject-working-tree
}
if [[ $(moose_is_git_submodule) ]]; then
    hookfile="$(cd $script_dir && git rev-parse --show-toplevel)/../.git/modules/moose/hooks/pre-commit"
else
    hookfile="$(cd $script_dir && git rev-parse --show-toplevel)/.git/hooks/pre-commit"
fi

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
cpp_file_paths="framework/src framework/include modules/*/src modules/*/include test unit examples tutorials stork"

changed_cpp_files=`git diff --staged --name-only -- $cpp_file_paths`
git clang-format -- $changed_cpp_files
git add $changed_cpp_files
' > $hookfile

chmod a+x $hookfile

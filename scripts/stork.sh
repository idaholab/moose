#!/bin/bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

function printusage {
    echo "Usage:    stork.sh <name>"
    echo ""
    echo "    Creates a new blank MOOSE app in the current working directory."
    echo "    <name> should be given in CamelCase format.  Allowed are letters and numbers "
    echo "           (cannot start with a number). No special characters are allowed."
    echo "    When --module is supplied after the <name> a MOOSE module will be created."
}

if [[ "$1" == "-h" || "$1" == "--help" || $# == 0 || $# > 2 ]]; then
    printusage
    exit 1
fi

if [[ $# == 2 && "$2" != "--module" ]]; then
    printusage
    exit 1
fi


if [[ $# == 2 && "$2" == "--module" ]]; then
    kind="module"
else
    kind="app"
fi

# set old/new app name variables
srcname='Stork'
dstname=$1

# check that dstname does not contain a special character
if [[ $dstname == *[\!\@\#\$\%\^\&\*\(\)\-\+]* ]] ; then
  echo "error: provided name contains a special character."
  exit 1
fi
# check that the name does not start with a number
if [[ ${dstname:0:1} == *[0-9]* ]] ; then
  echo "error: the first character of the name is a number."
  exit 1
fi

regex='s/([A-Z][a-z])/_\1/g; s/([a-z])([A-Z])/\1_\2/g; s/^_//;'

srcnamelow=$(echo "$srcname" | perl -pe "${regex}"'tr/[A-Z]/[a-z]/')
srcnameup=$(echo "$srcname" | perl -pe "${regex}"'tr/[a-z]/[A-Z]/')
dstnamelow=$(echo "$dstname" | perl -pe "${regex}"'tr/[A-Z]/[a-z]/')
dstnameup=$(echo "$dstname" | perl -pe "${regex}"'tr/[a-z]/[A-Z]/')
dstnamespace=$(echo "$dstname" | sed 's/\([^[:blank:]]\)\([A-Z]\)/\1 \2/g')
dir="$dstnamelow"
if [[ -z $MOOSE_DIR ]]; then
    MOOSE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )/.."
fi
if [[ "$kind" == "module" ]]; then
    dir="${MOOSE_DIR}/modules/$dir"
    if [[ ! -w "${MOOSE_DIR}/modules" ]] ; then
        echo "error: cannot create module without write permissions to $dir" ;
        exit 1
    fi
fi
if [[ ! -f "$MOOSE_DIR/stork/include/base/StorkApp.h" ]]; then
    echo "error: cannot find usable MOOSE directory"
    exit 1
fi

if [[ "$kind" == "app" ]]; then
    git status &>/dev/null && echo "error: your current working directory is inside a git repository" >&2 && exit 1
fi

# copy stork tree - abort if dir already exists or copy fails
if [[ -d "$dir" ]]; then
    echo "error: directory '$dir' already exists" >&2
    exit 1
fi
cp -R "$MOOSE_DIR/stork" "$dir" || echo "error: app/module creation failed" >&2 || exit 1

find $dir -type f -name '.*' | xargs rm -f # remove hidden files (e.g. vim swp files)

# rename app name within files
function recursiveRename {
    src=$1
    dst=$2
    grep --recursive -l "$src" $dir | xargs sed -i.bak 's/'"$src"'/'"$dst"'/g'
    find $dir -type f -name '*.bak' | xargs rm -f
}
recursiveRename "$srcname" "$dstname"
recursiveRename "$srcnamelow" "$dstnamelow"
recursiveRename "$srcnameup" "$dstnameup"

# rename files
mv "$dir/Makefile.${kind}" "$dir/Makefile"
mv "$dir/unit/Makefile.${kind}" "$dir/unit/Makefile"
mv "$dir/run_tests.${kind}" "$dir/run_tests"
mv "$dir/src/base/${srcname}App.C.${kind}" "$dir/src/base/${dstname}App.C"
mv "$dir/test/src/base/${srcname}TestApp.C.${kind}" "$dir/test/src/base/${dstname}TestApp.C"
mv "$dir/include/base/${srcname}App.h" "$dir/include/base/${dstname}App.h"
mv "$dir/test/include/base/${srcname}TestApp.h" "$dir/test/include/base/${dstname}TestApp.h"
mv "$dir/doc/config.yml.${kind}" "$dir/doc/config.yml"
mv "$dir/doc/sqa_reports.yml.${kind}" "$dir/doc/sqa_reports.yml"
mv "$dir/doc/moosedocs.py.${kind}" "$dir/doc/moosedocs.py"
chmod a+x "$dir/doc/moosedocs.py"
chmod a+x "$dir/run_tests"

# remove unnecessary files
rm -f $dir/Makefile.*
rm -f $dir/unit/Makefile.*
rm -f $dir/run_tests.*
rm -f $dir/src/base/StorkApp.C.*
rm -f $dir/test/src/base/StorkTestApp.C.*
rm -f $dir/doc/config.yml.*
rm -f $dir/doc/sqa_reports.yml.*
rm -f $dir/doc/moosedocs.py.*

if [[ "$kind" == "app" ]]; then
    # copy clang-format related files
    mkdir -p $dir/scripts
    cp $MOOSE_DIR/.clang-format $dir/
    cp $MOOSE_DIR/.gitignore $dir/

    # add application-specific generated resource file to end of gitignore file
    echo "$dstnamelow.yaml" >> $dir/.gitignore

    dir="$PWD/$dir"
    (cd $dir && git init && git add * .clang-format .gitignore && git commit -m "Initial files" && git branch -m main)

    echo "MOOSE app created in '$dir'"
    echo ""
    echo "To store your changes on GitHub:"
    echo "    1. Log in to your GitHub account"
    echo "    2. Create a new repository named '$dstnamelow'"
    echo "    3. In this terminal window, run the following commands:"
    echo "         cd $dir"
    echo "         git remote add origin https://github.com/YourGitHubUserName/$dstnamelow"
    echo '         git commit -m "Initial code commit"'
    echo "         git push -u origin main"
    echo ""
    echo "To automatically enforce MOOSE C++ code style in your commits, run:"
    echo ""
    echo "    cd $dir"
    echo "    ./scripts/install-format-hook.sh"
    echo ""
    echo "To enable software quality assurance (SQA) documentation using MooseDocs, perform the"
    echo "following steps after adding your git repository remote:"
    echo ""
    echo "    1. Navigate to $dir/doc"
    echo "    2. Run './moosedocs.py init sqa --app '$dstnamespace' --category '$dstnamelow'"
    echo "    3. Commit the initial SQA changes using the following commands:"
    echo "         git add $dir/doc"
    echo '         git commit -m "Initial SQA changes"'
    echo "         git push origin main"
    echo "    4. Add new SQA content to the forms in $dir/doc/content/sqa"
    echo ""
    echo "For general assistance in MOOSE-based application SQA, please contact the MOOSE"
    echo "framework development team. For further info on the MooseDocs code documentation"
    echo "system, please visit https://mooseframework.inl.gov."
fi

if [[ "$kind" == "module" ]]; then
    echo "New Module created in moose/modules"
    echo ""
    echo "There are several more steps that need to be completed"
    echo "    1. Modify the moose/modules/modules.mk file"
    echo "      a. Add the new module to the ALL_MODULES list (alphabetical)"
    echo "      b. Add the new module to the MODULE_NAMES variable (alphabetical)"
    echo "      c. Create a new registration section for the new module"
    echo "    2. Modify the moose/scripts/sqa_stats.py file"
    echo "      a. Add a new compute requirements stats entry for the module"
    echo "    3. Modify the moose/modules/combined/src/base/CombinedApp.C file"
    echo "      a. Add the new module to the set of included files (alphabetical)"
    echo "      b. Add the new module to the registerAll function (alphabetical)"
    echo "    4. Modify the moose/modules/doc/config.yml file"
    echo "       a. Add the module to the content listing (alphabetical)"
    echo "    5. Initialize module SQA (reach out to the MOOSE development team with questions)"
    echo "       a. Navigate to moose/modules/$dstnamelow/doc"
    echo "       b. Run './moosedocs.py init sqa --module '$dstnamespace' --category '$dstnamelow'"
    echo "       c. Add new module to SQA extension categories in modules/doc/config.yml (alphabetical)"
    echo "       d. Add new module to Applications and Requirements sections of modules/doc/sqa_reports.yml (alphabetical)"
    echo "    6. Ensure that no stork files hang around before committing"
    echo "    7. Ensure that proper testing is performed for per-module tests (e.g. parallel, recover testing)"
    echo ""

    rm -f $dir/LICENSE
    rm -f $dir/README.md
    rm -f $dir/scripts/*
    rmdir $dir/scripts
    rm -f $dir/run_tests
    ln -s ../../scripts/run_tests $dir/run_tests
fi

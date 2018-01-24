#!/bin/bash

function printusage {
    echo "Usage:    stork.sh <name>"
    echo ""
    echo "    Creates a new blank MOOSE app in the current working directory."
    echo "    <name> should be given in CamelCase format."
    echo "    When --module is supplied after the <name> a MOOSE module will be created."
}

if [[ "$1" == "-h" || $# == 0 || $# > 2 ]]; then
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

regex='s/([A-Z][a-z])/_\1/g; s/([a-z])([A-Z])/\1_\2/g; s/^_//;'

srcnamelow=$(echo "$srcname" | perl -pe "${regex}"'tr/[A-Z]/[a-z]/')
srcnameup=$(echo "$srcname" | perl -pe "${regex}"'tr/[a-z]/[A-Z]/')
dstnamelow=$(echo "$dstname" | perl -pe "${regex}"'tr/[A-Z]/[a-z]/')
dstnameup=$(echo "$dstname" | perl -pe "${regex}"'tr/[a-z]/[A-Z]/')
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

find $dir | grep '/[.]' | xargs rm -f # remove hidden files (e.g. vim swp files)

# rename app name within files
function recursiveRename {
    src=$1
    dst=$2
    grep --recursive -l "$src" $dir | xargs sed -i.bak 's/'"$src"'/'"$dst"'/g'
    find $dir | grep '\.bak$' | xargs rm -f
}
recursiveRename "$srcname" "$dstname"
recursiveRename "$srcnamelow" "$dstnamelow"
recursiveRename "$srcnameup" "$dstnameup"

# rename files
mv "$dir/Makefile.${kind}" "$dir/Makefile"
mv "$dir/run_tests.${kind}" "$dir/run_tests"
mv "$dir/src/base/${srcname}App.C.${kind}" "$dir/src/base/${dstname}App.C"
mv "$dir/test/src/base/${srcname}TestApp.C" "$dir/test/src/base/${dstname}TestApp.C"
mv "$dir/include/base/${srcname}App.h" "$dir/include/base/${dstname}App.h"
mv "$dir/test/include/base/${srcname}TestApp.h" "$dir/test/include/base/${dstname}TestApp.h"
chmod a+x "$dir/run_tests"

# remove unnecessary files
rm -f $dir/Makefile.*
rm -f $dir/run_tests.*
rm -f $dir/src/base/StorkApp.C.*

if [[ "$kind" == "app" ]]; then
    # copy clang-format related files
    mkdir -p $dir/scripts
    cp $MOOSE_DIR/.clang-format $dir/
    cp $MOOSE_DIR/.gitignore $dir/

    dir="$PWD/$dir"
    (cd $dir && git init && git add * .clang-format .gitignore && git commit -m"Initial files")

    echo "MOOSE app created in '$dir'"
    echo ""
    echo "To store your changes on github:"
    echo "    1. log in to your account"
    echo "    2. Create a new repository named $dstname"
    echo "    3. in this terminal run the following commands:"
    echo "         cd $dir"
    echo "         git remote add origin https://github.com/YourGitHubUserName/$dstname"
    echo '         git commit -m "initial commit"'
    echo "         git push -u origin master"
    echo ""
    echo "To automatically enforce MOOSE C++ code style in your commits, run:"
    echo ""
    echo "    cd $dir"
    echo "    ./scripts/install-format-hook.sh"
    echo ""
fi

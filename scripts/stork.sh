#!/bin/bash

function printusage {
    echo "Usage:    stork.sh <type> <AppName>"
    echo ""
    echo "    Creates a new blank MOOSE app in the current working directory."
    echo "    <type> must be either "app" or "module"."
    echo "    <AppName> should be given in CamelCase format."
}

if [[ "$1" == "-h" || $# != 2 ]]; then
    printusage
    exit 1
fi

if [[ "$1" != "app" && "$1" != "module" ]]; then
    echo "error: invalid type given" >&2
    printusage
    exit 1
fi

# set old/new app name variables
srcname='Stork'
dstname=$2
kind=$1
srcnamelow=$(echo "$srcname" | awk '{print tolower($0)}')
srcnameup=$(echo "$srcname" | awk '{print toupper($0)}')
dstnamelow=$(echo "$dstname" | awk '{print tolower($0)}')
dstnameup=$(echo "$dstname" | awk '{print toupper($0)}')
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

absdir=$(echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")")
if [[ "$kind" == "app" && "$absdir" =~ "$MOOSE_DIR" ]]; then
    echo "error: your current working directory is inside the MOOSE directory" >&2
    exit 1
fi

# make new app dir and copy stork files
if [[ -d "$dir" ]]; then
    echo "error: directory '$dir' already exists" >&2
    exit 1
fi
cp -R "$MOOSE_DIR/stork" "$dir"
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
mv "$dir/include/base/${srcname}App.h" "$dir/include/base/${dstname}App.h"
chmod a+x "$dir/run_tests"

# remove unnecessary files
rm -f $dir/Makefile.*
rm -f $dir/run_tests.*
rm -f $dir/src/base/StorkApp.C.*

if [[ "$kind" == "module" ]]; then
    rm -f "$dir/include/base/${dstname}App.h"
fi

if [[ "$kind" == "app" ]]; then
    (
        cd $dir
        git init
        git add *
    )
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
fi


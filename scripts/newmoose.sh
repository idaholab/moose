#!/bin/bash

function printusage {
    echo "Usage:    newapp.sh <type> <AppName>"
    echo ""
    echo "    <type> must be either "app" or "module"."
    echo "    <AppName> should be given in CapCase format."
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
fi

# make new app dir and copy stork files
if [[ -d "$dir" ]]; then
    echo "error: directory '$dir' already exists" >&2
    exit 1
fi
cp -R "$MOOSE_DIR/stork" "$dir"
find $dir | grep '/[.]' | xargs rm -rf # remove hidden files (e.g. vim swp files)

# rename app name within files
function recursiveRename {
    src=$1
    dst=$2
    grep --recursive -l "$src" $dir | xargs sed -i '' 's/'"$src"'/'"$dst"'/g'
}
recursiveRename "$srcname" "$dstname"
recursiveRename "$srcnamelow" "$dstnamelow"
recursiveRename "$srcnameup" "$dstnameup"

# rename files
mv "$dir/Makefile.${kind}" "$dir/Makefile"
mv "$dir/run_tests.${kind}" "$dir/run_tests"
mv "$dir/src/base/${srcname}App.C.${kind}" "$dir/src/base/${dstname}App.C"
mv "$dir/include/base/${srcname}App.h" "$dir/include/base/${dstname}App.h"

# remove unnecessary files
rm -f "$dir/Makefile.*"
rm -f "$dir/run_tests.*"
rm -f "$dir/src/base/StorkApp.C.*"

if [[ "$kind" == "module" ]]; then
    rm -f "$dir/include/base/${dstname}App.h"
fi


#!/bin/bash

codeurl='https://github.com/idaholab/stork'
codeversion='master'

if [[ "$1" == "-h" || $# != 1 ]]; then
    echo "Usage:    newapp.sh YourAppName"
    exit 0
fi

if [[ -z $1 ]]; then
    echo "error: no app name given." >&2
    echo "Usage:    newapp.sh YourAppName"
    exit 1
fi

# set old/new app name variables
srcname='Stork'
dstname=$1
srcnamelow=$(echo "$srcname" | awk '{print tolower($0)}')
srcnameup=$(echo "$srcname" | awk '{print toupper($0)}')
dstnamelow=$(echo "$dstname" | awk '{print tolower($0)}')
dstnameup=$(echo "$dstname" | awk '{print toupper($0)}')
dir="$dstnamelow"

# download stork files
function download {
    curl -L "$codeurl/tarball/$codeversion" | tar zx
}
download || download || download # retry download up to two times
if [[ "$?" != "0" ]]; then
    echo "failed to download MOOSE stork assets" >&2
    exit 0
fi
mv idaholab-stork-* $dstnamelow

# rename files and code to new app name
function recursiveRename {
    src=$1
    dst=$2
    grep --recursive -l "$src" $dir | xargs sed -i '' 's/'"$src"'/'"$dst"'/g'
}
recursiveRename $srcname $dstname
recursiveRename $srcnamelow $dstnamelow
recursiveRename $srcnameup $dstnameup

mv "$dir/Makefile.app" "$dir/Makefile"
mv "$dir/run_tests.app" "$dir/run_tests"
mv "$dir/include/base/${srcname}App.h" "$dir/include/base/${dstname}App.h"
mv "$dir/src/base/${srcname}App.C.app" "$dir/src/base/${dstname}App.C"

# remove unnecessary files
rm "$dir/Makefile.module"
rm "$dir/run_tests.module"
rm "$dir/src/base/StorkApp.C.module"
rm "$dir/make_new_application.py"
rm "$dir/make_new_module.py"


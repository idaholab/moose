#!/usr/bin/env bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR/..

# Test for git repository
git_dir=`git rev-parse --show-cdup 2>/dev/null`
if [[ "x$git_dir" == "x" ]]; then
    git submodule update --init large_media
    if [[ $? != 0 ]]; then
        echo "git submodule command failed, are your proxy settings correct?"
        exit 1
    fi
fi
exit 0

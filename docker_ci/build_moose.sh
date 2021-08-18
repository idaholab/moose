#!/bin/bash

function make_pwd()
{
  for method in $METHODS; do
    METHOD=$method make -j $MOOSE_JOBS $*
  done
}

cd framework
make_pwd

cd ../test
make_pwd

if [[ -n "$BUILD_MODULES" ]]; then
  cd ../modules
  make_pwd all builds
fi

#!/bin/bash
set -eu

git clean -xfd
git submodule update --init moose
cp -R pyhit $SP_DIR/
cd src
make bindings
cp hit.so $SP_DIR/pyhit/
cat > $SP_DIR/pyhit-$PKG_VERSION.egg-info <<FAKE_EGG
Metadata-Version: 2.1
Name: pyhit
Version: $PKG_VERSION
Summary: MOOSE HIT Parser library
Platform: UNKNOWN
FAKE_EGG

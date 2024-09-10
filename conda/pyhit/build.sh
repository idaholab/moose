#!/bin/bash
set -eu

cp -R pyhit $SP_DIR/
cd src

printf '\n\n\nDEBUG BUILDING\n'
printf 'FLAGS?\n%s\n\n' "$LDFLAGS"

make bindings

printf '\n\n\nWHAT ARE THE LINKS\n'
ldd hit.so
printf '\n\n\n'

cp hit.so $SP_DIR/pyhit/
cat > $SP_DIR/pyhit-$PKG_VERSION.egg-info <<FAKE_EGG
Metadata-Version: 2.1
Name: pyhit
Version: $PKG_VERSION
Summary: MOOSE HIT Parser library
Platform: UNKNOWN
FAKE_EGG

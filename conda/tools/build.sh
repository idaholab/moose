#!/bin/bash
set -eu
install -d $PREFIX/share
install -m 644 moose-python-deps $PREFIX/share/moose-tools

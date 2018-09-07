#!/bin/bash
MOOSE_DIR=$(realpath "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"/..)
(cd $MOOSE_DIR/framework/contrib/hit && make &>/dev/null)
find ${MOOSE_DIR} -name '*.i' | xargs ${MOOSE_DIR}/framework/contrib/hit/hit format -i $@ $files



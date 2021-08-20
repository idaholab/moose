#!/usr/bin/env bash
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script is intended for INTERNAL use only by CIVET testing.
#
# It is recommended for all users to use our default version of PETSc,
# which can be compiled using scripts/update_and_rebuild_petsc.sh
#
# If you wish to use an alternate version of PETSc, you should compile
# it on your own and set PETSC_DIR and PETSC_ARCH.

# Give regular users a big warning
echo "*** WARNING ***"
echo "scripts/update_and_rebuild_petsc_alt.sh is intended for internal"
echo "use only. Please use scripts/update_and_rebuild_petsc.sh instead."

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo $SCRIPT_DIR

cd $SCRIPT_DIR/..

# Initialize petsc submodule
git_dir=`git rev-parse --show-cdup 2>/dev/null`
if [[ $? == 0 && "x$git_dir" == "x" ]]; then
  git submodule update --init --recursive petsc
  if [[ $? != 0 ]]; then
    echo "git submodule command failed, are your proxy settings correct?"
    exit 1
  fi
fi

# Jump into PETSc dir
cd $SCRIPT_DIR/../petsc
# I would like to control this number without touching civet recipe.
# In the future, if I want to use another version of PETSc such as 3.12.0
# as petsc-alt, I only need to change v3.11.4 to v3.12.0.
# My change will need to go through as a MOOSE PR, and then everything will be
# tested.
git checkout v3.11.4
if [[ $? != 0 ]]; then
  echo "git checkout command failed, are your proxy settings correct?"
  exit 1
fi

# Back to script dir
cd $SCRIPT_DIR

# --skip-submodule-update is used so that we will stay with the particular version
# we checkout earlier. hypre is relocated, so we do a customized overwrite for v3.11.4
sh $SCRIPT_DIR/update_and_rebuild_petsc.sh $* \
  --skip-submodule-update \
  --download-strumpack=0 \
  --download-hypre=git://https://github.com/hypre-space/hypre.git \
  --with-64-bit-indices=0 \
exit 0

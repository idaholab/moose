#!/bin/bash
set -eu

# In order to set CXXFLAGS during moose-mpich activation with the proper C++
# standard, we first extract what mpich-mpicxx gives us during build. Just in case
# the clang or gcc conda-forge packages set a standard, remove any occurence of
# "-std=c++XX"
TEMP_CXXFLAGS=${CXXFLAGS//-std=c++[0-9][0-9]}
# The  "-fdebug-prefix-map" flags then need to be removed, as they are specific
# to the conda build process. Finally, `-std=c++17` can be appended to the end.
ACTIVATION_CXXFLAGS=${TEMP_CXXFLAGS%%-fdebug-prefix-map*}-std=c++17

# Set MPICH environment variables for those that need it, and set CXXFLAGS using our ACTIVATION_CXXFLAGS variable
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77 C_INCLUDE_PATH=${PREFIX}/include MOOSE_NO_CODESIGN=true MPIHOME=${PREFIX} CXXFLAGS="$ACTIVATION_CXXFLAGS" HDF5_DIR=${PREFIX}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset CC CXX FC F90 F77 C_INCLUDE_PATH MOOSE_NO_CODESIGN MPIHOME CXXFLAGS HDF5_DIR
EOF

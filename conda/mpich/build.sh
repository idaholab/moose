#!/bin/bash
set -eu

# In order to set a CXXFLAGS during moose-mpich activation with the proper C++
# standard (as clang and gcc from conda-forge still default to C++14), we extract
# what moose_cxx gives us here, before we unset it. The  "-fdebug-prefix-map"
# flags are also removed, as they are specific to the conda build process.
TEMP_CXXFLAGS=${CXXFLAGS%%-fdebug-prefix-map*}-std=c++17
# Finally, swap "-std=c++14" with "" to set our basic standard requirement.
ACTIVATION_CXXFLAGS=${TEMP_CXXFLAGS/-std=c++14/}

# Set MPICH environment variables for those that need it, and set CXXFLAGS using our ACTIVATION_CXXFLAGS variable
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77 C_INCLUDE_PATH=${PREFIX}/include MOOSE_NO_CODESIGN=true MPIHOME=${PREFIX} CXXFLAGS="${ACTIVATION_CXXFLAGS}"
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset CC CXX FC F90 F77 C_INCLUDE_PATH MOOSE_NO_CODESIGN MPIHOME CXXFLAGS
EOF

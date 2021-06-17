#!/bin/bash
set -eu
# Allow mpirun/exec to oversubscribe without errors
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<'EOF' > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export MOOSE_NO_CODESIGN=true
if [ `uname` = "linux" ]; then
  export FC=${FC:-"x86_64-conda_cos6-linux-gnu-gfortran"}
  export CC=${CC:-"x86_64-conda_cos6-linux-gnu-cc"} CXX=${CXX:-"x86_64-conda_cos6-linux-gnu-c++"} F90=${F90:-$FC} F77=${F77:-$FC}
else
  export FC=${FC:-"x86_64-apple-darwin13.4.0-gfortran"}
  export CC=${CC:-"x86_64-apple-darwin13.4.0-clang"} CXX=${CXX:-"x86_64-apple-darwin13.4.0-clang++"} F90=${F90:-$FC} F77=${F77:-$FC}
fi
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset MOOSE_NO_CODESIGN CCACHE_SLOPPINESS CC CXX FC F90 F77
EOF

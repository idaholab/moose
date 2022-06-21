#!/bin/bash
set -ex
export GOPATH="$PREFIX/pprof"
# Build/Install pprof from google at specified hash
go install github.com/google/pprof@c488b8fa1db3fa467bf30beb5a1d6f4f10bb1b87
# Set GPERF_DIR path (influential environment variable in MOOSE make files)
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export GPERF_DIR=${PREFIX}
export PPROF_OLDPATH=\${PATH}
export PATH=${PREFIX}/pprof/bin:\${PATH}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset GPERF_DIR
export PATH=\${PPROF_OLDPATH}
EOF

#!/bin/bash
set -ex
export GOPATH="$PREFIX/pprof"
# Build/Install pprof from google at specified hash
go install github.com/google/pprof@latest
# go creates read-only files. Do this so Civet can properly clean up
chmod -R 700 ${GOPATH}/pkg
rm -rf ${GOPATH}/pkg

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

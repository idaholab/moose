#!/bin/bash

set -ex

# Get an updated config.sub and config.guess
cp $BUILD_PREFIX/share/gnuconfig/config.* .
if [ `uname` == "Darwin" ]; then
    TEMP_CXXFLAGS=${CXXFLAGS//-std=c++[0-9][0-9]}
    # gperftools uses random_shuffle (removed in c++17)
    ACTIVATION_CXXFLAGS=${TEMP_CXXFLAGS%%-fdebug-prefix-map*}-std=c++14
    export CFLAGS="$CFLAGS -D_XOPEN_SOURCE"
    ./configure  --prefix $PREFIX --enable-libunwind --disable-dependency-tracking
else
    ./configure  --prefix $PREFIX --enable-libunwind --enable-frame-pointers
fi
CORES=${MOOSE_JOBS:-2}
make -j $CORES
make install
# Remove unwanted pprof
mv $(which pprof) $(dirname $(which pprof))/original_pprof
export GOPATH="$PREFIX/pprof"
# Build/Install pprof from google
go install github.com/google/pprof@latest
cd $PREFIX/bin
ln -s ../pprof/bin/pprof .

# Set GPERF_DIR path (influential environment variable in MOOSE make files)
mkdir -p "${PREFIX}/etc/conda/activate.d" "${PREFIX}/etc/conda/deactivate.d"
cat <<EOF > "${PREFIX}/etc/conda/activate.d/activate_${PKG_NAME}.sh"
export GPERF_DIR=${PREFIX}
EOF
cat <<EOF > "${PREFIX}/etc/conda/deactivate.d/deactivate_${PKG_NAME}.sh"
unset GPERF_DIR
EOF

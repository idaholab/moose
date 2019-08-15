FROM ubuntu

WORKDIR /opt

#-----------------------------------------------------------------------------#
# Install aptitutde packages and clear cache
#-----------------------------------------------------------------------------#
RUN export DEBIAN_FRONTEND=noninteractive ; \
apt-get update -y ; \
apt-get install -y \
build-essential \
gfortran \
tcl \
git \
m4 \
freeglut3 \
doxygen \
libblas-dev \
liblapack-dev \
libx11-dev \
libnuma-dev \
libcurl4-gnutls-dev \
zlib1g-dev \
libhwloc-dev \
python \
python-dev \
cmake \
curl ; \
rm -rf /var/lib/apt/lists/*

#-----------------------------------------------------------------------------#
# Install mpich-3.3 to system path
#-----------------------------------------------------------------------------#
RUN curl -L -O http://www.mpich.org/static/downloads/3.3/mpich-3.3.tar.gz ; \
tar -xf mpich-3.3* ; \
cd mpich-3.3 && mkdir gcc-build && cd gcc-build ; \
# Configure build env
../configure --prefix=/usr/local \
--enable-shared \
--enable-sharedlibs=gcc \
--enable-fast=O2 \
--enable-debuginfo \
--enable-totalview \
--enable-two-level-namespace \
CC=gcc \
CXX=g++ \
FC=gfortran \
F77=gfortran \
F90='' \
CFLAGS='' \
CXXFLAGS='' \
FFLAGS='' \
FCFLAGS='' \
F90FLAGS='' \
F77FLAGS='' ; \
# Build and install
make -j $(grep -c ^processor /proc/cpuinfo) ; \
make install ; \
# Cleanup
cd ../../ ; rm -rf mpich-3.3*

#-----------------------------------------------------------------------------#
# Set environment variables for MOOSE deps
#-----------------------------------------------------------------------------#
ENV CC=mpicc \
CXX=mpicxx \
PETSC_DIR=/usr/local \
LIBMESH_DIR=/usr/local \
MOOSE_DIR=/opt/moose

#-----------------------------------------------------------------------------#
# Copy MOOSE repository into the image
#-----------------------------------------------------------------------------#
COPY . ${MOOSE_DIR}
WORKDIR ${MOOSE_DIR}

#-----------------------------------------------------------------------------#
# Install submodule dependencies
#-----------------------------------------------------------------------------#
RUN export MOOSE_JOBS=$(grep -c ^processor /proc/cpuinfo) ; \
# Install petsc to system path
./scripts/update_and_rebuild_petsc.sh ; \
rm -rf petsc/* ; \
# Install libmesh to system path
./scripts/update_and_rebuild_libmesh.sh ; \
rm -rf libmesh/* ; \
# Build moose tests including framework
cd test; make -j $MOOSE_JOBS 
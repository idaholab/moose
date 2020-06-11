#-----------------------------------------------------------------------------#
# This Dockerfile builds MPICH, PETSc, Libmesh, and finally MOOSE in a way that
# is designed to take advantage of Docker's build caching.  Nominally, one 
# would clone MOOSE, then run the build scripts.  This approach would result in
# unnecessary rebuilding of upstream dependencies due to changes that only 
# involve MOOSE, leading to unneccessary, redundant use of computating 
# resources, bandwidth, and storage.
#
# To build a MOOSE image, there is one optional argument and two required 
# arguments to be set using the --build-arg flag.  
#
# DISTRO_NAME: Optional.  Linux distribution (ubuntu, centos)
# DISTRO_VERSION: Optional with above (18.04, 7 respectively)
# MOOSE_JOBS: Optional.  Sets number of cores for running make.
# PETSC_REV: Commit hash of submodule petsc.
# LIBMESH_REV: Commit hash of submodule libmesh.
#-----------------------------------------------------------------------------#
ARG DISTRO_NAME=ubuntu
ARG DISTRO_VERSION=18.04

FROM ${DISTRO_NAME}:${DISTRO_VERSION}

WORKDIR /opt

ARG MOOSE_JOBS=1

#-----------------------------------------------------------------------------#
# Install managed packages and clear cache
#-----------------------------------------------------------------------------#

COPY docker_ci/*.sh ./

RUN chmod +x *.sh ; \
./install_packages.sh

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
make -j ${MOOSE_JOBS} ; \
make install ; \
# Cleanup
cd ../../ ; rm -rf mpich-3.3*

ENV CC=mpicc \
CXX=mpicxx \
MOOSE_DIR=/opt/moose

WORKDIR ${MOOSE_DIR}

#-----------------------------------------------------------------------------#
# Install PETSc to system path
#-----------------------------------------------------------------------------#
ARG PETSC_REV
COPY scripts/update_and_rebuild_petsc.sh ${MOOSE_DIR}/scripts/update_and_rebuild_petsc.sh
RUN git clone https://gitlab.com/petsc/petsc.git ; \
cd petsc && git checkout ${PETSC_REV} && cd .. ; \
PETSC_PREFIX=/usr/local ./scripts/update_and_rebuild_petsc.sh ; \
rm -rf petsc/* petsc/.* || true

ENV PETSC_DIR=/usr/local

#-----------------------------------------------------------------------------#
# Install Libmesh to system path
#-----------------------------------------------------------------------------#
ARG LIBMESH_REV
ENV LIBMESH_DIR=/usr/local \
libmesh_CPPFLAGS="-D LIBMESH_HAVE_XDR"
COPY scripts/update_and_rebuild_libmesh.sh ${MOOSE_DIR}/scripts/update_and_rebuild_libmesh.sh
RUN git clone https://github.com/libMesh/libmesh.git ; \
cd libmesh ; \
git checkout ${LIBMESH_REV} ; \
git submodule update --init ; \
cd .. ; \
./scripts/update_and_rebuild_libmesh.sh ; \
rm -rf libmesh/* libmesh/.* || true

#-----------------------------------------------------------------------------#
# Copy and build MOOSE framework and tests
#-----------------------------------------------------------------------------#
COPY . ${MOOSE_DIR}
RUN cd test ; make -j ${MOOSE_JOBS}

#-----------------------------------------------------------------------------#
# Add needed env vars to /etc/environment
#-----------------------------------------------------------------------------#
RUN for CURR_VAR in $(env | grep 'CC\|CXX\|_DIR'); do \
    echo "$CURR_VAR" >> /etc/environment ; \
done

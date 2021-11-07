#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

#-----------------------------------------------------------------------------#
# This Dockerfile builds MPICH, PETSc, Libmesh, and finally MOOSE in a way that
# is designed to take advantage of Docker's build caching.  Nominally, one
# would clone MOOSE, then run the build scripts.  This approach would result in
# unnecessary rebuilding of upstream dependencies due to changes that only
# involve MOOSE, leading to unneccessary, redundant use of computing
# resources, bandwidth, and storage.
#
# To build a MOOSE image, there are eight optional arguments and two required
# arguments to be set using the --build-arg flag.
#
# DISTRO_NAME: Optional.  Linux distribution (ubuntu, centos)
# DISTRO_VERSION: Optional with above (20.04, 8 respectively)
# LIBMESH_METHODS: Optional. Methods with which to build libmesh; defaults to opt dbg.
# MOOSE_JOBS: Optional.  Sets number of cores for running make.
# MOOSE_METHODS: Optional. Methods with which to build MOOSE; defaults to opt.
# PETSC_REV: Commit hash of submodule petsc.
# PETSC_OPTIONS: Optional. Options to pass to update_and_rebuild_petsc.sh.
# LIBMESH_REV: Commit hash of submodule libmesh.
# LIBMESH_OPTIONS: Optional. Options to pass to update_and_rebuild_libmesh.sh.
# BUILD_MODULES: Optional. Set to build all MOOSE modules.
#-----------------------------------------------------------------------------#
ARG DISTRO_NAME=ubuntu
ARG DISTRO_VERSION=20.04

FROM ${DISTRO_NAME}:${DISTRO_VERSION}

WORKDIR /opt

ARG MOOSE_JOBS=1

#-----------------------------------------------------------------------------#
# Add user dev
#-----------------------------------------------------------------------------#
RUN useradd dev ; \
mkdir -p /home/dev/.ssh ; \
chmod 700 /home/dev/.ssh ; \
chown -R dev:dev /home/dev

#-----------------------------------------------------------------------------#
# Install managed packages and clear cache
#-----------------------------------------------------------------------------#
COPY docker_ci/install_packages.sh docker_ci/apt_installs.sh docker_ci/yum_installs.sh ./
RUN ./install_packages.sh; rm *.sh

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
ARG PETSC_OPTIONS
COPY scripts/update_and_rebuild_petsc.sh ${MOOSE_DIR}/scripts/update_and_rebuild_petsc.sh
COPY scripts/configure_petsc.sh ${MOOSE_DIR}/scripts/configure_petsc.sh
RUN git clone https://gitlab.com/petsc/petsc.git ; \
cd petsc && git checkout ${PETSC_REV} && cd .. ; \
PETSC_PREFIX=/usr/local ./scripts/update_and_rebuild_petsc.sh ${PETSC_OPTIONS} ; \
rm -rf petsc/* petsc/.* || true

ENV PETSC_DIR=/usr/local

#-----------------------------------------------------------------------------#
# Install Libmesh to system path
#-----------------------------------------------------------------------------#
ARG LIBMESH_REV
ARG LIBMESH_OPTIONS
ARG LIBMESH_METHODS=opt dbg
ENV LIBMESH_DIR=/usr/local \
libmesh_CPPFLAGS="-D LIBMESH_HAVE_XDR"

COPY scripts/update_and_rebuild_libmesh.sh ${MOOSE_DIR}/scripts/update_and_rebuild_libmesh.sh
COPY scripts/configure_libmesh.sh ${MOOSE_DIR}/scripts/configure_libmesh.sh

RUN git clone https://github.com/libMesh/libmesh.git ; \
cd libmesh ; \
git checkout ${LIBMESH_REV} ; \
git submodule update --init ; \
cd .. ; \
METHODS="${LIBMESH_METHODS}" ./scripts/update_and_rebuild_libmesh.sh ${LIBMESH_OPTIONS} ; \
rm -rf libmesh/* libmesh/.* || true

#-----------------------------------------------------------------------------#
# Copy and build MOOSE framework, test, and optionally modules
#-----------------------------------------------------------------------------#
RUN chown -R dev:dev /opt /home/dev

USER dev
COPY --chown=dev:dev . ${MOOSE_DIR}

ARG BUILD_MODULES
ARG MOOSE_METHODS=opt
RUN ./docker_ci/build_moose.sh

#-----------------------------------------------------------------------------#
# Add needed env vars to /etc/environment
#-----------------------------------------------------------------------------#
USER root
RUN for CURR_VAR in $(env | grep 'CC\|CXX\|_DIR'); do \
    echo "$CURR_VAR" >> /etc/environment ; \
done

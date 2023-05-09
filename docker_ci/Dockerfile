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
# Optional arguments (set with --build-arg):
# DISTRO_NAME: Linux distribution; defults to ubuntu.
# DISTRO_VERSION: Linux distribution version; defaults to 20.04 with ubuntu.
# LIBMESH_METHODS: Methods with which to build libmesh; defaults to "opt dbg".
# MOOSE_JOBS: Sets number of cores for running make; defaults to 1.
# MOOSE_METHODS: Methods with which to build MOOSE; defaults to opt.
# PETSC_REV: Commit hash of submodule petsc.
# PETSC_OPTIONS: Options to pass to update_and_rebuild_petsc.sh.
# LIBMESH_REV: Commit hash of submodule libmesh.
# LIBMESH_OPTIONS: Options to pass to update_and_rebuild_libmesh.sh.
# BUILD_MODULES: Set to build all MOOSE modules.
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
RUN ./install_packages.sh || exit 1; rm *.sh

#-----------------------------------------------------------------------------#
# Ensure all local repos are marked as safe for all users
#-----------------------------------------------------------------------------#
RUN for CURR_USR in root dev; do \
    sudo -u $CURR_USR /bin/bash -c "printf '[safe]\n\tdirectory = *\n' > ~/.gitconfig" ; \
done

#-----------------------------------------------------------------------------#
# Install mpich-4.0.2 to system path
#-----------------------------------------------------------------------------#
RUN curl -L -O https://www.mpich.org/static/downloads/4.0.2/mpich-4.0.2.tar.gz ; \
tar -xf mpich-4.0.2.tar.gz ; \
cd mpich-4.0.2 && mkdir gcc-build && cd gcc-build ; \
# Configure build env
../configure --prefix=/usr/local \
--enable-shared \
--enable-sharedlibs=gcc \
--enable-fast=O2 \
--enable-debuginfo \
--enable-totalview \
--with-device=ch3 \
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
cd ../../ ; rm -rf mpich-4.0.2 ; \
# Ensure mpich was installed by checking for binary
if [ ! -f $(which mpicc) ]; then exit 1; fi

ENV CC=mpicc \
CXX=mpicxx \
MOOSE_DIR=/opt/moose

WORKDIR ${MOOSE_DIR}

#-----------------------------------------------------------------------------#
# Install PETSc to system path
#-----------------------------------------------------------------------------#
ARG PETSC_REV=f855b95493736b087b8ccc16dc6c5b29bc4b5aa8
ARG PETSC_OPTIONS
ENV PETSC_DIR=/usr/local
COPY scripts/update_and_rebuild_petsc.sh ${MOOSE_DIR}/scripts/update_and_rebuild_petsc.sh
COPY scripts/configure_petsc.sh ${MOOSE_DIR}/scripts/configure_petsc.sh
RUN git clone https://gitlab.com/petsc/petsc.git ; \
cd petsc && git checkout ${PETSC_REV} && cd .. ; \
PETSC_PREFIX=$PETSC_DIR ./scripts/update_and_rebuild_petsc.sh ${PETSC_OPTIONS} --download-cmake ; \
rm -rf petsc/* petsc/.* || true ; \
# Ensure PETSc was installed based on present of library directory
if [ ! -d $PETSC_DIR/lib/petsc ]; then exit 1; fi

#-----------------------------------------------------------------------------#
# Install Libmesh to system path
#-----------------------------------------------------------------------------#
ARG LIBMESH_REV=5e720a226ba3740dbe92c135936612461f489ada
ARG LIBMESH_OPTIONS
ARG LIBMESH_METHODS="opt dbg"
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
rm -rf libmesh/* libmesh/.* || true ; \
# Ensure Libmesh was installed via include directory
if [ ! -d $LIBMESH_DIR/include/libmesh ]; then exit 1; fi

#-----------------------------------------------------------------------------#
# Install wasp to system path
#-----------------------------------------------------------------------------#
ARG WASP_REV=827479b89fb5b76a0865974d9f4ca404d55acce0
ENV WASP_DIR=/usr/local/wasp

COPY scripts/update_and_rebuild_wasp.sh ${MOOSE_DIR}/scripts/update_and_rebuild_wasp.sh
COPY scripts/configure_wasp.sh ${MOOSE_DIR}/scripts/configure_wasp.sh

RUN mkdir -p framework/contrib ; \
cd framework/contrib ; \
git clone https://code.ornl.gov/neams-workbench/wasp.git ; \
cd wasp ; \
git checkout ${WASP_REV} ; \
git submodule update --init --recursive ; \
cd ../../.. ; \
./scripts/update_and_rebuild_wasp.sh -D CMAKE_INSTALL_PREFIX:STRING=${WASP_DIR} ; \
rm -rf framework || true

#-----------------------------------------------------------------------------#
# Copy and build MOOSE framework, test, and optionally modules
#-----------------------------------------------------------------------------#
RUN chown -R dev:dev /opt /home/dev

USER dev
COPY --chown=dev:dev . ${MOOSE_DIR}

ARG BUILD_MODULES
ARG MOOSE_METHODS=opt
RUN ./docker_ci/build_moose.sh ; \
# Ensure a MOOSE test binary was built
if [ ! -f $MOOSE_DIR/test/moose_test* ]; then exit 1; fi ; \
git submodule init

#-----------------------------------------------------------------------------#
# Add needed env vars to /etc/environment
#-----------------------------------------------------------------------------#
USER root

RUN : > /etc/environment ; \
for CURR_VAR in $(env | grep '^CC\|^CXX\|_DIR\|^PATH'); do \
    echo "$CURR_VAR" >> /etc/environment ; \
done

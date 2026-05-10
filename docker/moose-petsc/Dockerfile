#
# moose-petsc docker image
#
# -- Required arguments -----------------------------------------------------------
#
# BASE_IMAGE: Base image to pull from
# PETSC_GIT_REF: Git reference for PETSc
#
# -- Useful optional arguments ----------------------------------------------------
#
# BUILD_JOBS: Number of jobs to build with
# PETSC_GIT_REMOTE: Remote for cloning PETSc
# PETSC_OPTIONS: Options for building PETSc
#
# -- Contains ---------------------------------------------------------------------
#
# petsc
#   location: PETSC_DIR=/opt/petsc
#   version: PETSC_GIT_REF arg (git reference)
#   extra options: PETSC_OPTIONS arg
#
# -- Consumes ---------------------------------------------------------------------
#
# docker/moose-petsc/setup_mpi.bash
#   For picking a MPI environment from the base, deleting any unused MPI installls
# docker/moose-petsc/build_petsc.bash
# scripts/configure_petsc.sh
# scripts/update_and_rebuild_petsc.sh
#   For building PETSc
#

ARG BASE_IMAGE=unset

FROM $BASE_IMAGE

ARG MPI_FLAVOUR=mpich
ARG MOOSE_ENV=/moose_env.sh

# Remove unused mpi
COPY docker/moose-petsc/setup_mpi.bash /root/setup_mpi.bash
RUN MOOSE_ENV=$MOOSE_ENV /bin/bash /root/setup_mpi.bash && rm /root/setup_mpi.bash

# Build arguments
ARG BUILD_DIR=/root/build
ARG BUILD_JOBS=4

# PETSc build options
ARG PETSC_GIT_REF=
ARG PETSC_OPTIONS=
ARG PETSC_GIT_REMOTE=https://gitlab.com/petsc/petsc.git
COPY docker/moose-petsc/build_petsc.bash scripts/configure_petsc.sh scripts/update_and_rebuild_petsc.sh ${BUILD_DIR}/
RUN PETSC_DIR="/opt/petsc" \
    PETSC_GIT_REMOTE="$PETSC_GIT_REMOTE" \
    PETSC_GIT_REF="$PETSC_GIT_REF" \
    PETSC_OPTIONS="$PETSC_OPTIONS" \
    BUILD_DIR="$BUILD_DIR" \
    BUILD_JOBS="$BUILD_JOBS" \
    MOOSE_ENV="$MOOSE_ENV" /bin/bash "${BUILD_DIR}/build_petsc.bash" \
    && rm -rf "$BUILD_DIR"

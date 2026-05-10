#
# moose-libmesh docker image
#
# -- Required arguments --------------------------------------------------------------------------
#
# BASE_IMAGE: Base image to pull from
# VTK_VERSION: Numeric version of VTK to use
# LIBMESH_GIT_REF: Git reference for libmesh
#
# -- Useful optional arguments -------------------------------------------------------------------
#
# BUILD_JOBS: Number of jobs to build with
# LIBMESH_GIT_REMOTE: Remote for cloning libmesh (default: https://github.com/libMesh/libmesh.git)
# LIBMESH_METHODS: Methods for building libmesh (default: opt dbg oprof devel)
#
# -- Contains ------------------------------------------------------------------------------------
#
# vtk
#   location: VTK_DIR=/opt/vtk
#   version: VTK_VERSION arg (numeric version)
# libmesh
#   location: LIBMESH_DIR=/opt/libmesh
#   version: LIBMESH_GIT_REF arg (git reference), from remote LIBMESH_GIT_REMOTE arg
#   extra options: LIBMESH_OPTIONS arg
#   methods: LIBMESH_METHODS arg
#
# -- Consumes -----------------------------------------------------------------------------------
#
# docker/moose-libmesh/build_vtk.bash
#   For building VTK
# docker/moose-libmesh/build_libmesh.bash
# scripts/configure_libmesh.sh
# scripts/update_and_rebuild_libmesh.sh
#   For building libMesh
#

ARG BASE_IMAGE=unset

FROM $BASE_IMAGE

ARG MOOSE_ENV=/moose_env.sh
RUN echo "# From moose-libmesh" >> $MOOSE_ENV

# Build arguments
ARG BUILD_DIR=/root/build
ARG BUILD_JOBS=4

# Build vtk
ARG VTK_VERSION=
COPY docker/moose-libmesh/build_vtk.bash ${BUILD_DIR}/build_vtk.bash
RUN VTK_VERSION="$VTK_VERSION" \
    VTK_DIR="/opt/vtk" \
    BUILD_DIR="$BUILD_DIR" \
    BUILD_JOBS="$BUILD_JOBS" \
    MOOSE_ENV="$MOOSE_ENV" /bin/bash "${BUILD_DIR}/build_vtk.bash" \
    && rm -rf "$BUILD_DIR"

# Build libmesh
ARG LIBMESH_GIT_REMOTE=https://github.com/libMesh/libmesh.git
ARG LIBMESH_GIT_REF=
ARG LIBMESH_OPTIONS=
ARG LIBMESH_METHODS="opt dbg oprof devel"
COPY docker/moose-libmesh/build_libmesh.bash scripts/configure_libmesh.sh scripts/update_and_rebuild_libmesh.sh ${BUILD_DIR}/
RUN LIBMESH_GIT_REF="$LIBMESH_GIT_REF" \
    GIT_REMOTE="$LIBMESH_GIT_REMOTE" \
    LIBMESH_OPTIONS="$LIBMESH_OPTIONS" \
    METHODS="$LIBMESH_METHODS" \
    BUILD_DIR="$BUILD_DIR" \
    MOOSE_JOBS="$BUILD_JOBS" \
    MOOSE_ENV="$MOOSE_ENV" /bin/bash "${BUILD_DIR}/build_libmesh.bash" \
    && rm -rf "$BUILD_DIR"

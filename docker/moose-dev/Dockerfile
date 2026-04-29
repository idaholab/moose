#
# moose-dev docker image
#
# -- Required arguments -----------------------------------------------------
#
# BASE_IMAGE: Base image to pull from
# WASP_GIT_REF: Git reference for WASP
# NEML2_GIT_REF: Git reference for NEML2 (if !LIBTORCH_SKIP and !NEML2_SKIP)
# CONDUIT_GIT_REF: Git reference for conduit (if !MFEM_SKIP)
# MFEM_GIT_REF: Git reference for MFEM (if !MFEM_SKIP)
#
# -- Useful optional arguments ----------------------------------------------
#
# BUILD_JOBS: Number of jobs to build with
# LIBTORCH_SKIP: Set to anything to skip building libtorch
# NEML2_SKIP: Set to anything to skip building NEML2
# MFEM_SKIP: Set to anything to skip building conduit and MFEM
#
# -- Contains ---------------------------------------------------------------
#
# hwloc-gui
#   location: system package
#   when: detected mpi is MPICH with HWLOC
# miniforge
#   location: /opt/miniforge
#   version: MINIFORGE_VERSION arg
# libtorch
#   location: LIBTORCH_DIR=/opt/libtorch
#   version: LIBTORCH_VERSION arg
#   disabled by LIBTORCH_SKIP arg
# code-server
#   location: system package
#   version: CODE_SERVER_VERSION arg
# moose language server code-server extension
#   location: /opt/code-server/extensions
#   version: MOOSE_LANGUAGE_SUPPORT_VERSION arg
# wasp
#   location: WASP_DIR=/opt/wasp
#   version: WASP_GIT_REF arg (git reference)
# neml2
#   location: NEML2_DIR=/opt/neml2
#   version: NEML2_GIT_REF arg (git reference)
#   disabled by NEML2_SKIP arg
# conduit
#   location: CONDUIT_DIR=/opt/conduit
#   version: CONDUIT_GIT_REF arg (git reference)
#   disabled by CONDUIT_SKIP arg
# mfem
#   location: MFEM_DIR=/opt/mfem
#   version: MFEM_GIT_REF arg (git reference)
#   disabled by CONDUIT_SKIP arg
# moose-tools conda environment
#   location: /opt/miniforge/envs/moose
#   python version: PYTHON_VERSION arg (default: 3.14)
#   moose-tools version: MOOSE_TOOLS_VERSION arg
#

ARG BASE_IMAGE=unset

FROM $BASE_IMAGE

ARG MOOSE_ENV=/moose_env.sh
RUN echo "# From moose-dev" >> $MOOSE_ENV

# Build arguments
ARG BUILD_DIR=/root/build
ARG BUILD_JOBS=4

# When using mpich with hwloc, install hwloc-gui to get
# lstopo-no-graphics, which can be used to prebuild a
# topology file to avoid hwloc reading topology repeatedly
RUN source /moose_env.sh \
    && if [ -d "$MOOSE_MPICH_DIR" ] && mpiexec --version | grep -q hwloc; then \
        dnf install -y hwloc-gui && dnf clean all; \
    fi

# Install miniforge
ARG MINIFORGE_VERSION=26.1.1-3
RUN MINIFORGE_DIR=/opt/miniforge3 \
    && mkdir $BUILD_DIR \
    && cd $BUILD_DIR \
    && curl -L https://github.com/conda-forge/miniforge/releases/download/${MINIFORGE_VERSION}/Miniforge3-${MINIFORGE_VERSION}-Linux-x86_64.sh -o miniforge.sh \
    && /bin/bash miniforge.sh -b -p $MINIFORGE_DIR \
    && rm miniforge.sh \
    && echo "export PATH=${MINIFORGE_DIR}/bin:\${PATH}" >> $MOOSE_ENV \
    && source $MOOSE_ENV \
    && conda update -yq --all \
    && conda clean -y --all

# Install libtorch
ARG LIBTORCH_SKIP=
ARG LIBTORCH_VERSION=2.11.0
COPY docker/moose-dev/build_libtorch.bash ${BUILD_DIR}/
RUN if [ -z "$LIBTORCH_SKIP" ]; then \
        LIBTORCH_VERSION=$LIBTORCH_VERSION \
        LIBTORCH_DIR=$LIBTORCH_DIR \
        BUILD_DIR=$BUILD_DIR \
        BUILD_JOBS=$BUILD_JOBS \
        MOOSE_ENV=$MOOSE_ENV \
        /bin/bash ${BUILD_DIR}/build_libtorch.bash; \
    fi && \
    rm -rf "$BUILD_DIR"

# Install code-server
ARG CODE_SERVER_VERSION=4.115.0
RUN CODE_SERVER_FILE=code-server-${CODE_SERVER_VERSION}-amd64.rpm \
    && mkdir $BUILD_DIR \
    && cd $BUILD_DIR \
    && curl -fOL https://github.com/coder/code-server/releases/download/v${CODE_SERVER_VERSION}/${CODE_SERVER_FILE} \
    && rpm -i ${CODE_SERVER_FILE} \
    && rm -rf $BUILD_DIR

# Setup directory for code-server extensions. The extensions folder can be
# filled by other applications as they wish to have extensions auto installed
# by the "setup-code-server-extensions" script (in PATH)
ARG MOOSE_LANGUAGE_SUPPORT_VERSION=1.5.2
COPY docker/moose-dev/opt/code-server /opt/code-server
RUN mkdir /opt/code-server/extensions \
    && cd /opt/code-server/extensions \
    && curl -L -O https://github.com/idaholab/moose-language-support/releases/download/v${MOOSE_LANGUAGE_SUPPORT_VERSION}/moose-language-support-${MOOSE_LANGUAGE_SUPPORT_VERSION}.vsix


# Install wasp
ARG WASP_GIT_REMOTE=https://code.ornl.gov/neams-workbench/wasp.git
ARG WASP_GIT_REF=
COPY docker/moose-dev/build_wasp.bash scripts/configure_wasp.sh scripts/update_and_rebuild_wasp.sh ${BUILD_DIR}/
RUN WASP_GIT_REF=$WASP_GIT_REF \
    GIT_REMOTE=$WASP_GIT_REMOTE \
    BUILD_DIR=$BUILD_DIR \
    BUILD_JOBS=$BUILD_JOBS \
    MOOSE_ENV=$MOOSE_ENV \
    /bin/bash ${BUILD_DIR}/build_wasp.bash \
    && rm -rf "$BUILD_DIR"

# Install neml2
ARG NEML2_GIT_REMOTE=https://github.com/applied-material-modeling/neml2.git
ARG NEML2_GIT_REF=
ARG NEML2_SKIP=
COPY docker/moose-dev/build_neml2.bash scripts/configure_neml2.sh scripts/update_and_rebuild_neml2.sh ${BUILD_DIR}/
RUN if [ -z "$NEML2_SKIP" ] && [ -z "$LIBTORCH_SKIP" ]; then \
        NEML2_GIT_REF=$NEML2_GIT_REF \
        GIT_REMOTE=$NEML2_GIT_REMOTE \
        BUILD_DIR=$BUILD_DIR \
        BUILD_JOBS=$BUILD_JOBS \
        MOOSE_ENV=$MOOSE_ENV \
        /bin/bash ${BUILD_DIR}/build_neml2.bash; \
    fi \
    && rm -rf "$BUILD_DIR"

# Install conduit
ARG MFEM_SKIP=
ARG CONDUIT_GIT_REMOTE=https://github.com/llnl/conduit.git
ARG CONDUIT_GIT_REF=
COPY docker/moose-dev/build_conduit.bash scripts/update_and_rebuild_conduit.sh ${BUILD_DIR}/
RUN if [ -z "$MFEM_SKIP" ]; then \
        CONDUIT_GIT_REF=$CONDUIT_GIT_REF \
        GIT_REMOTE=$CONDUIT_GIT_REMOTE \
        BUILD_JOBS=$BUILD_JOBS \
        BUILD_DIR=$BUILD_DIR \
        BUILD_JOBS=$BUILD_JOBS \
        MOOSE_ENV=$MOOSE_ENV \
        /bin/bash ${BUILD_DIR}/build_conduit.bash; \
    fi \
    && rm -rf "$BUILD_DIR"

# Install mfem
ARG MFEM_GIT_REMOTE=https://github.com/mfem/mfem.git
ARG MFEM_GIT_REF=
COPY docker/moose-dev/build_mfem.bash scripts/update_and_rebuild_mfem.sh ${BUILD_DIR}/
RUN if [ -z "$MFEM_SKIP" ]; then \
        MFEM_GIT_REF=$MFEM_GIT_REF \
        GIT_REMOTE=$MFEM_GIT_REMOTE \
        BUILD_JOBS=$BUILD_JOBS \
        BUILD_DIR=$BUILD_DIR \
        BUILD_JOBS=$BUILD_JOBS \
        MOOSE_ENV=$MOOSE_ENV \
        /bin/bash ${BUILD_DIR}/build_mfem.bash; \
    fi \
    && rm -rf "$BUILD_DIR"

# Install moose-tools conda package
ARG CONDA_ALT_CHANNEL=
ARG MOOSE_TOOLS_VERSION=
ARG PYTHON_VERSION=3.14
RUN source $MOOSE_ENV \
    && conda config --env --add channels https://conda.software.inl.gov/public \
    && test -z $CONDA_ALT_CHANNEL || conda config --env --add channels ${CONDA_ALT_CHANNEL} \
    && conda create -yq -n moose python=$PYTHON_VERSION moose-tools=$MOOSE_TOOLS_VERSION \
    && . $(dirname $(which conda))/../etc/profile.d/conda.sh && conda activate moose \
    && pip3 --no-cache install lcov-cobertura \
    && conda clean -ya \
    && echo ". $(dirname $(which conda))/../etc/profile.d/conda.sh" >> $MOOSE_ENV \
    && echo "conda activate moose" >> $MOOSE_ENV

# TODO: move this lower down later
RUN ln -s /moose_env.sh /etc/profile.d/moose_env.sh && chmod +x /moose_env.sh

#
# moose-petsc docker image
#
# -- Required arguments -------------------------------
#
# BASE_IMAGE: Base image to pull from
#
# -- Contains -----------------------------------------
#
# System packages from docker/moose-mpi/installs.bash
#
# -- Consumes -----------------------------------------
#
# docker/moose-mpi/installs.bash
#   For base installs
#

ARG BASE_IMAGE=unset
FROM $BASE_IMAGE

ARG BUILD_DIR=/root/build

COPY docker/moose-mpi/installs.bash ${BUILD_DIR}/
RUN /bin/bash ${BUILD_DIR}/installs.bash && rm -rf "$BUILD_DIR"

# Test MPI
RUN source /moose_env.sh \
    && ${MOOSE_MPICH_DIR}/bin/mpiexec --version \
    && (test -z $MOOSE_OPENMPI_DIR || (${MOOSE_OPENMPI_DIR}/bin/mpiexec --version && ${MOOSE_OPENMPI_DIR}/bin/ompi_info))

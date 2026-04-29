#!/bin/bash
set -euxo pipefail

# Source environment to get MOOSE_[MPICH,OPENMPI]_DIR
set +u
source "${MOOSE_ENV:?}"
set -u

# Header for environment file
echo "# From moose-petsc-${MPI_FLAVOUR}" >> "$MOOSE_ENV"

if [ "$MPI_FLAVOUR" = "mpich" ]; then
    test -n "${MOOSE_MPICH_DIR:?}"
    echo "source ${MOOSE_MPICH_DIR}/env.sh" >> "$MOOSE_ENV"
    if [ -n "${MOOSE_OPENMPI_DIR:-}" ]; then
        rm -rf "$MOOSE_OPENMPI_DIR"
        echo "unset MOOSE_OPENMPI_DIR" >> "$MOOSE_ENV"
    fi
elif [ "$MPI_FLAVOUR" = "openmpi" ]; then
    test -n "${MOOSE_OPENMPI_DIR:?}"
    echo "source ${MOOSE_OPENMPI_DIR}/env.sh" >> "$MOOSE_ENV"
    if [ -n "${MOOSE_MPICH_DIR:-}" ]; then
        rm -rf "$MOOSE_MPICH_DIR"
        echo "unset MOOSE_MPICH_DIR" >> "$MOOSE_ENV"
    fi
else
    echo "Unknown MPI_FLAVOUR=${MPI_FLAVOUR}"
    exit 1
fi

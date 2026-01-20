# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Provide capabilities for determining the run-time MPI implementation."""

import os
from enum import Enum
from shutil import which
from subprocess import CalledProcessError, check_output


class MPIConfig(Enum):
    """Enum that represents the MPI implementations that we special case."""

    UNKNOWN = 0
    OPENMPI = 1
    MPICH_NO_HWLOC = 2
    MPICH_WITH_HWLOC = 3


@staticmethod
def get_mpi_config() -> MPIConfig:
    """Get the MPI configuration for the implementations that we special case."""
    mpi_command = os.environ.get("MOOSE_MPI_COMMAND", "mpiexec")
    which_mpiexec = which(mpi_command)
    if which_mpiexec is None:
        return MPIConfig.UNKNOWN

    try:
        out = check_output([mpi_command, "--version"], text=True)
    except CalledProcessError:
        return MPIConfig.UNKNOWN

    if "HYDRA" in out:
        if "hwloc" in out:
            return MPIConfig.MPICH_WITH_HWLOC
        else:
            return MPIConfig.MPICH_NO_HWLOC
    if "Open MPI" in out:
        return MPIConfig.OPENMPI

    return MPIConfig.UNKNOWN

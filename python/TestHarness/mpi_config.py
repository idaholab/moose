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
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from shutil import which
from socket import gethostname
from subprocess import CalledProcessError, check_output, run
from traceback import format_exc
from typing import Optional


class MPIType(Enum):
    """Enum that represents the MPI implementations that we special case."""

    UNKNOWN = 0
    OPENMPI = 1
    MPICH = 2


@dataclass
class MPIConfig:
    mpi_type: MPIType
    hwloc: bool
    hwloc_topo_file: Optional[str]


def build_hwloc_topo() -> Optional[str]:
    """Build the hwloc topology file, if possible."""
    exe = which("lstopo-no-graphics")
    if exe is None:
        return None

    try:
        store_dir = os.path.join(Path.home(), ".local", "share", "moose", "testharness")
        if not os.path.exists(store_dir):
            os.makedirs(store_dir, exist_ok=True)
        store_file = os.path.join(store_dir, f"hwloc_topo_{gethostname()}.xml")
        cmd = [exe, "--of", "xml", "-f", store_file]
        run(cmd, check=True)
        return store_file
    except Exception:
        print("WARNING: Failed to build hwloc topology file")
        print(format_exc())

    return None


@staticmethod
def get_mpi_config() -> MPIConfig:
    """Get the MPI configuration for the implementations that we special case."""
    config = MPIConfig(mpi_type=MPIType.UNKNOWN, hwloc=False, hwloc_topo_file=None)

    mpi_command = os.environ.get("MOOSE_MPI_COMMAND", "mpiexec").split()[0]
    which_mpiexec = which(mpi_command)
    if which_mpiexec is not None:
        try:
            out = check_output([mpi_command, "--version"], text=True)
        except CalledProcessError:
            pass
        else:
            if "HYDRA" in out:
                config.mpi_type = MPIType.MPICH
                if "hwloc" in out:
                    config.hwloc = True
            elif "Open MPI" in out:
                config.mpi_type = MPIType.OPENMPI

    if config.hwloc:
        config.hwloc_topo_file = build_hwloc_topo()

    return config

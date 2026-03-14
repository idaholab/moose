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
from subprocess import PIPE, STDOUT, CalledProcessError, check_output, run
from traceback import format_exc
from typing import Optional


class MPIType(Enum):
    """Enum that represents the MPI implementations that we special case."""

    UNKNOWN = 0
    OPENMPI = 1
    MPICH = 2


@dataclass(frozen=True)
class MPIConfig:
    """The found MPI configuration."""

    mpi_type: MPIType
    """The MPI type found, if any."""
    hwloc: bool
    """Whether or not the found MPI uses hwloc."""


def build_hwloc_topology() -> Optional[str]:
    """
    Build the hwloc topology file, if possible.

    If we're able to build this once up front (instead of
    on every single application execution), we can save a
    sizeable amount of time on init.
    """
    if (exe := which("lstopo-no-graphics")) is None and (
        exe := which("lstopo")
    ) is None:
        return None

    try:
        store_dir = os.path.join(Path.home(), ".local", "share", "moose", "testharness")
        os.makedirs(store_dir, exist_ok=True)
    except Exception:
        print(f"WARNING: Failed to setup .local directory:\n{format_exc()}\n")
        return None

    store_file = os.path.join(store_dir, f"hwloc_topo_{gethostname()}.xml")
    cmd = [exe, "--of", "xml", "-f", store_file]
    process = run(cmd, stderr=STDOUT, stdout=PIPE, check=False, text=True)
    if process.returncode != 0:
        print(f"WARNING: Failed to build hwloc topology file:\n{process.stdout}\n")
        return None

    return store_file


def get_mpi_config() -> MPIConfig:
    """Get the MPI configuration for the implementations that we special case."""
    mpi_type = MPIType.UNKNOWN
    hwloc = False

    mpi_command = os.environ.get("MOOSE_MPI_COMMAND", "mpiexec").split()[0]
    which_mpiexec = which(mpi_command)
    if which_mpiexec is not None:
        try:
            out = check_output([mpi_command, "--version"], text=True)
        except CalledProcessError:
            pass
        else:
            if "HYDRA" in out:
                mpi_type = MPIType.MPICH
                if "hwloc" in out:
                    hwloc = True
            elif "Open MPI" in out:
                mpi_type = MPIType.OPENMPI
                hwloc = True

    return MPIConfig(mpi_type=mpi_type, hwloc=hwloc)

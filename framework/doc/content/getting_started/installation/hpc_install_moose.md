# HPC Cluster

The following instructions aims at setting up a baseline single-user environment for building MOOSE based applications in a job scheduling capable environment.

If instead, you are here interested in allowing MOOSE-based development to be made avaialable to multiple users, please see our [Multi-User](getting_started/installation/cluster.md) setup instructions (requires administrative rights). Normal users: please ingore the previous sentence, and continue on...

## Pre-Reqs

What ever compiler you choose to use on your cluster (GCC/Clang, MPICH/OpenMPI), the minimum requirement, is that it must be C++11 compatible. If you are unsure, please consult with your system admins for your cluster on which compiler to use (and how to use it).

- +CMake+. A modern version of CMake (>2.8) is required to build some of the meta packages we need to include in PETSc.

- +Python 2.7.x Development libraries+.

Your cluster will most likely have these two requirements available via some form of environment management software. If you are unfamiliar with how to manage your environment, please consult with your cluster administrators.

## PREFIX Setup

```bash
export STACK_SRC=`mktemp -d /tmp/stack_temp.XXXXXX`
export PACKAGES_DIR=$HOME/moose-compilers
```

!alert! note
- The `$PACKAGES_DIR` directory must reside in a location where all the compute nodes can access.
- We need the above two environment variables to exist throughout these instructions. Please use the one and only one, terminal you executed those two commands with.
!alert-end!

!include manual_petsc.md

## Create MOOSE Profile

Use an editor to add the following contents to `$HOME/.moose_profile`

!package! code max-height=400
PACKAGES_DIR=$HOME/moose-compilers

export CC=mpicc
export CXX=mpicxx
export F90=mpif90
export F77=mpif77
export FC=mpif90

export PETSC_DIR=$PACKAGES_DIR/petsc-__PETSC_DEFAULT__
!package-end!

## Source the MOOSE Profile

```bash
source $HOME/.moose_profile
```

By sourcing the above file, you are now ready to begin MOOSE-based development.

!alert note title=+Remember to source the profile!+
You will need to perform the above (`source $HOME/.moose_profile`) for every new terminal session for which you perform work with MOOSE. If you want this to be automatic, add the above to your `~/.bash_profile` (or `~/.bashrc` or, which ever profile you use on your system)

!include manual_cleanup.md

Head back over to the [getting_started/index.md] page to continue your tour of MOOSE.

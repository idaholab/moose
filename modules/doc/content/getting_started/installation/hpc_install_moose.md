# HPC Cluster

The following instructions aims at setting up a baseline single-user environment for building MOOSE
based applications in a job scheduling capable environment.

If you are interested in allowing MOOSE-based development to be made available to multiple users,
please see our [Multi-User](getting_started/installation/cluster.md) setup instructions (requires
administrative rights).

## Pre-Reqs

!include sqa/minimum_requirements.md

- +CMake+. A modern version of CMake (>3.5) is required to build some of the meta packages we need to include in PETSc.

- +Python 3.x Development libraries+.

Your cluster will most likely have these requirements available via some form of environment
management software. If you are unfamiliar with how to manage your environment or unsure how to
obtain the above requirements, please consult with your cluster administrators.

## Create MOOSE Profile

Use an editor to add the following contents to `$HOME/.moose_profile`

!package! code max-height=400

export CC=mpicc
export CXX=mpicxx
export F90=mpif90
export F77=mpif77
export FC=mpif90

!package-end!

## Source the MOOSE Profile

```bash
source $HOME/.moose_profile
```

By sourcing the above file, you are now ready to begin MOOSE-based development.

!alert note title=+Remember to source the profile!+
You will need to perform the above (`source $HOME/.moose_profile`) for every new terminal session
for which you perform work with MOOSE. If you want this to be automatic, add the above to your
`~/.bash_profile` (or `~/.bashrc` or, which ever profile you use on your system)

!include getting_started/installation/install_moose.md

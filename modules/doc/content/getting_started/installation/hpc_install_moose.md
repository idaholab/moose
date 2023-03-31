# HPC Cluster

The following instructions are for those operating on an HPC cluster.

!alert note
It is entirely possible your cluster may have too old of libraries. If this ends up being the case
after failing the below instructions, please follow our regular [Linux](installation/conda.md)
instructions instead for an easier, but lower performance, installation.

## Prerequisites

!include sqa/minimum_requirements.md

- CMake. A modern version of CMake is required to build some of the meta packages we need to include
  in PETSc
- Python 3.x Development libraries

Your cluster will most likely have these requirements available via some form of environment
management software. If you are unfamiliar with how to manage your environment or unsure how to
obtain the above requirements, please consult with your cluster administrators.

## Activate Environment

!style! halign=left
Activate your desired MPI environment (refer to your cluster administrators on how to do this).
This *usually* involves `module load` commands. Please note again, that Intel compilers are not
supported.
!style-end!

Sometimes after loading a proper MPI environment, it is still necessary to set some variables.
Check to see if the following variables are set:

```bash
echo $CC $CXX $FC $F90 $F77
```

If nothing returns, or what does return does *not* include MPI naming conventions (`$CC` is `gcc`
and not `mpicc` like we need), you need to set them manually each and every time you load said
environment:

```bash
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77
```

!include installation/start_up_profile.md

## Cloning MOOSE

!style! halign=left
!template load file=installation/clone_moose.md.template PATH=~/projects
!style-end!

## PETSc and libMesh

!style! halign=left
MOOSE requires several support libraries in order to build or run properly. Both of these libraries
(PETSc and libMesh) can be built using our supplied scripts:
!style-end!

!template load file=installation/build_petsc_and_libmesh.md.template PATH=~/projects

## Build and Test MOOSE

!style! halign=left
!template load file=installation/build_moose.md.template PATH=~/projects
!style-end!

!template load file=installation/test_moose.md.template PATH=~/projects

Now that you have a working MOOSE, and you know how to make your MPI wrapper available, proceed to
'New Users' to begin your tour of MOOSE!

!content pagination use_title=True
                    previous=installation/hpc_index.md
                    next=getting_started/hpc_new_users.md

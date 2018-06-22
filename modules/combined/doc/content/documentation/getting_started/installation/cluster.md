# HPC Cluster Setup

The following document aims at setting up a baseline multi-user environment for building MOOSE based
applications in a job scheduling capable environment.

## Prerequisites

Both of these pre-reqs are in the hands of the admins of the cluster.

- Modules. If not already installed. ['Modules Environment'](http://modules.sourceforge.net/) (Or
  some kind of environment management software)
- Whatever compiler you choose to use on your cluster (GCC/Clang/Intel, MPICH/OpenMPI/MVAPICH), the
  minimum requirement, is that it must be C++11 compatible. If you are unsure, please consult with
  your system admins for your cluster on which compiler to use (and how to use it).

## Environment Setup

- Begin by creating an area for which to build.

```bash
export CLUSTER_TEMP=`mktemp -d /tmp/cluster_temp.XXXXXX`
cd $CLUSTER_TEMP
```

!alert note
The terminal you used to run that command, should be the terminal you use from now on while following the instructions to completion

## Set your umask

Some systems have a secure umask set. We need to adjust our umask so that everything we are about
to perform, is readable/executable by _<b>everyone</b>_ on your cluster:

```bash
umask 0022
```

## Choose a base path

Export a base path variable which will be the home location for the compiler stack. All files
related to MOOSE will be stored in this location (so choose carefully now):

```bash
export PACKAGES_DIR=/opt/moose-compilers
```

!alert warning
You can change this path to whatever you want. The only exception, is this path must reside in a
location where all your compute nodes have access (nfs|panfs share)

## Setup Modules

Even if you are not using Modules, the following step should give you some idea as to what environment variables are needed for MOOSE developement.

Create a MOOSE module:

```bash
sudo mkdir -p $PACKAGES_DIR/modulefiles
sudo vi $PACKAGES_DIR/modulefiles/moose-dev-gcc
```

Add the following content to that file:

!package! code
#%Module1.0#####################################################################
##
## MOOSE module

##
set base_path   INSERT PACKAGES_DIR HERE

GCC MPI PATHS

setenv CC       mpicc
setenv CXX      mpicxx
setenv F90      mpif90
setenv F77      mpif77
setenv FC       mpif90

setenv          PETSC_DIR       $base_path/petsc/petsc-__PETSC_DEFAULT__/gcc-opt
!package-end!

!alert! note
Replace `INSERT PACKAGES_DIR HERE` with whatever you had set for $PACKAGES_DIR.

Replace `GCC MPI PATHS` with any additional information needed to make GCC/MPI work.
!alert-end!

To make the module available in your terminal session, export the following:

```bash
export MODULEPATH=$MODULEPATH:$PACKAGES_DIR/modulefiles
```

The above should be performed using a more permanent method so that it is usable by everyone at anytime (including while on a node).

## Building PETSc

If you opted to build a moose-dev-gcc module, and if you have modules available on your cluster, we will load it as a user would. This will ensure that everything we did above is going to work for our end-users.

```bash
module load moose-dev-gcc
echo $PETSC_DIR
```

!alert note
Verify that the above `echo $PETSC_DIR` command is returning /the-packages_dir-path/petsc/petsc-!!package petsc_default!! you originally setup during the 'Choose a base path' step. If not, something went wrong with creating the moose-dev-gcc module in the previous step.

If you chose not to create a moose-dev-gcc module, you will instead need to export the PETSC_DIR manually before continuing:

!package! code
export PETSC_DIR=$PACKAGES_DIR/petsc/petsc-__PETSC_DEFAULT__/gcc-opt
!package-end!

Download and extract PETSc:

!package! code
curl -L -O http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-__PETSC_DEFAULT__.tar.gz
tar -xf petsc-__PETSC_DEFAULT__.tar.gz
cd petsc-__PETSC_DEFAULT__
!package-end!

Configure PETSc using the following options:

!include getting_started/petsc_default.md

During the configure/build process, you will be prompted to enter proper make commands. This can be different from system to system, so I leave that task to the reader.

## Clean Up

Clean all the temporary stuff:

```bash
rm -rf $CLUSTER_TEMP
```

This concludes setting up the environment for MOOSE development. While you, the admin of your cluster, may not be developing MOOSE based applications, it would be wise to proceed with the following steps, to insure the environment this document has helped you create, is working as intended.

!include getting_started/installation/clone_moose.md

!include getting_started/installation/build_libmesh.md

!include getting_started/installation/test_moose.md

!include getting_started/installation/create_an_app.md

!include getting_started/installation/update_moose.md

# Multi-User Cluster Instructions

This document will aid an +HPC Administrator+ on building an environment which multiple end-users will use when compiling and running MOOSE based applications. +If you do not have administrative rights, you will not be able to complete these instructions!+ Please forward these instructions to your HPC Administrator.

Because this document assumes the reader *is* an administrator, most of the content herein are suggestions, and not 'copy, paste, enter' instructions.

## Prerequisites

- Some sort of environmental module management software. Such as [Environment Modules](http://modules.sourceforge.net/).
- A working MPI wrapper (MPICH/OpenMPI/MVAPICH) which wraps to a C++17 compliant compiler.
- Knowledge on Access Control Lists (ACLs) or other means to safeguard a directory from further tampering by others.

!alert warning
Please use a single solitary terminal session throughout and to the completion of these instructions.

## Environment Setup

Begin by creating an area for which to build:

```bash
export STACK_SRC=`mktemp -d /tmp/stack_src_temp.XXXXXX`
cd $STACK_SRC
```

## Set your umask

Some systems have a secure umask set. We need to adjust our umask so that when you write a file (`make install`), it is readable by everyone:

```bash
umask 0022
```

## Choose a base path

Export a base path variable which will be the home location for the compiler stack. All files related to libraries necessary to build MOOSE, will be stored in this location (+choose carefully, as this location should be accessible from all nodes on your cluster+):

```bash
export PACKAGES_DIR=/opt/moose
```

## Create and chown $PACKGES_DIR

History teaches us, that implicitly trusting scripts we download off the internet with root access, is a very bad idea. So let us create and chown the $PACKAGES_DIR directory before we install anything. That way, the things we do install can be done so without invoking `sudo`:

```bash
sudo mkdir -p $PACKAGES_DIR
sudo chown -R <your user id> $PACKAGES_DIR
```

!alert! warning
Verify that your umask settings are indeed set to 0022 before continuing:

```bash
$> umask
0022
```
!alert-end!

## Set Up Modules

Even if you are not using Modules, the following provides information on what environment variables are needed for MOOSE developement.

Create a MOOSE module:

```bash
mkdir -p $PACKAGES_DIR/modulefiles
vi $PACKAGES_DIR/modulefiles/moose-dev-gcc
```

Add the following content to that file:

!package! code max-height=500
#%Module1.0#####################################################################
##
## MOOSE module

##
set base_path   INSERT PACKAGES_DIR HERE

GCC MPI PATHS

prepend-path    PATH             /GCC and MPI /bin
prepend-path    LD_LIBRARY_PATH  /GCC and MPI /lib

setenv CC       mpicc
setenv CXX      mpicxx
setenv F90      mpif90
setenv F77      mpif77
setenv FC       mpif90

setenv          PETSC_DIR        $base_path/petsc/petsc-__PETSC_DEFAULT__

# Optional if miniconda is installed
prepend-path    PATH             $base_path/miniconda/bin
!package-end!

!alert! note
Replace `INSERT PACKAGES_DIR HERE` with whatever you had set for $PACKAGES_DIR (+do not+ literally enter: $PACKAGES_DIR. As an example, if you left packages_dir as: /opt/moose, then that is what you would enter)

Replace `GCC and MPI` paths with any additional information needed to make GCC/MPI work on your cluster.
!alert-end!

To make the module available in your terminal session, export the following:

```bash
export MODULEPATH=$MODULEPATH:$PACKAGES_DIR/modulefiles
```

The above command should be added to the list of other global profiles (perhaps in /etc/profiles.d). That way, the above is performed as the user logs into the machine.

With the modulefile in place and the MODULEPATH variable set, see if our module is available for loading:

```bash
module load moose-dev-gcc
```

Verify that this module loads properly by attempting to echo $PETSC_DIR:

!package! code
echo $PETSC_DIR
/opt/moose/petsc/petsc-__PETSC_DEFAULT__
!package-end!

While we are at it, verify that your MPI wrapper works by running a few commands (your results will vary, but they should return something):

```bash
which mpicc
/opt/moose/mpich-3.2/gcc-7.3.0/bin/mpicc

mpicc -show
gcc -I/opt/moose/mpich-3.2/gcc-7.3.0/include -L/opt/moose/mpich-3.2/gcc-7.3.0/lib -Wl,-rpath -Wl,/opt/moose/mpich-3.2/gcc-7.3.0/lib -Wl,--enable-new-dtags -lmpi

which gcc
/opt/moose/gcc-7.3.0/bin/gcc
```

Leave this module loaded for the remainder of the instructions (PETSc requirements).

!include getting_started/installation/manual_petsc.md

!include getting_started/installation/manual_miniconda.md

## Clean Up and Chown

Clean all the temporary stuff and change the ownership to root, so no further writes are possible:

```bash
rm -rf $STACK_SRC
sudo chown -R root:root $PACKAGE_DIR
```

This concludes setting up the environment for MOOSE-based development. However you decide to instruct your users on enabling the above environment, each user will need to perform the instructions provided by the following link: [Obtaining and Building MOOSE](getting_started/installation/install_moose.md).

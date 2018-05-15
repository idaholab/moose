# Manual Installation

## Prerequisites

- Cmake 3.4 or greater will be needed for building PETSc. Unless your system is very old, one should
  be able to use their system's package manager (apt-get, yum, zypper, etc) to install a compatible
  version of Cmake. For older systems, you will need to obtain cmake source from http://www.cmake.org,
  and build it appropriately for your system.

- A sane environment. This means having a clean, nothing but the bare minimum as far as available
  libraries go in your running environment. No additional LD_LIBRARY_PATHs, or other extra PATHs
  set. No strange UMASK settings. No odd aliases. It might even be best, to create a separate
  account, strictly for the use of these instructions. I have created an account called 'moose', and
  will assume you have done the same.

## Environment

Lets try to make our environment as sane as possible, while setting up all the locations we will need.

```bash
module purge   #(may fail with command not found)
unset LD_LIBRARY_PATH
unset CPLUS_INCLUDE_PATH
unset C_INCLUDE_PATH

export CC=gcc
export CXX=g++
export FC=gfortran
export F90=gfortran
export PACKAGES_DIR=/opt/moose
export STACK_SRC=/tmp/moose_stack_src
umask 022
```

!alert note
What ever terminal window you were in, when you performed the above exports and umask commands, you
_MUST_ use that same window, for the remainder of these instructions. If this window is closed, or
the machine is rebooted, it will be necessary to perform the above commands again, before continuing
any step. You will also _need_ to perform any exports in any previous steps you continued from.


And now we create our target installation location. We will also `chown` the location to our own user
id for now. This will allow us to perform all the `make install` commands with out the need of sudo,
which can complicate things.

```bash
mkdir -p $STACK_SRC
sudo mkdir -p $PACKAGES_DIR
sudo chown -R moose $PACKAGES_DIR
```

## GCC

We need a modern C++11 capable compiler. Our minimum requirements are: GCC 4.8.4, Clang 3.4.0,
and Intel20130607. This document will focus on building a GCC 7.3.0 compiler stack.

What version of GCC do we have?

```bash
gcc --version

gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-4)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

If your version is less than 4.8.4, you will need to build a newer version. If your version is at or
greater than 4.8.4, you have the option of skipping this section, and moving on to the MPICH section
further down.

```bash
cd $STACK_SRC
curl -L -O http://mirrors.concertpass.com/gcc/releases/gcc-7.3.0/gcc-7.3.0.tar.gz
tar -xf gcc-7.3.0.tar.gz -C .
```

Obtain GCC pre-reqs:

```bash
cd $STACK_SRC/gcc-7.3.0
./contrib/download_prerequisites
```

Configure, build and install GCC:

```bash
mkdir $STACK_SRC/gcc-build
cd $STACK_SRC/gcc-build

../gcc-7.3.0/configure --prefix=$PACKAGES_DIR/gcc-7.3.0 \
--disable-multilib \
--enable-languages=c,c++,fortran,jit \
--enable-checking=release \
--enable-host-shared \
--with-pic

make -j #   (where # is the number of cores available)

make install
```

Any errors during configure/make will need to be investigated on your own. Every operating system I
have come across has its own nuances of getting stuff built. Normally any issues are going to be
solved by installing the necessary development libraries using your system package manager (apt-get,
yum, zypper, etc). Hint: I would search the internet for 'how to build GCC 5.4.0 on (insert the
name/version of your operating system here)'

!alert! note
In order to utilize our newly built GCC 7.3.0 compiler, we need to set some variables:

```bash
export PATH=$PACKAGES_DIR/gcc-7.3.0/bin:$PATH
export LD_LIBRARY_PATH=$PACKAGES_DIR/gcc-7.3.0/lib64:$PACKAGES_DIR/gcc-7.3.0/lib:$PACKAGES_DIR/gcc-7.3.0/lib/gcc/x86_64-unknown-linux-gnu/7.3.0:$PACKAGES_DIR/gcc-7.3.0/libexec/gcc/x86_64-unknown-linux-gnu/7.3.0:$LD_LIBRARY_PATH
```
!alert-end!

## MPICH

Download MPICH 3.2

```bash
cd $STACK_SRC
curl -L -O http://www.mpich.org/static/downloads/3.2/mpich-3.2.tar.gz
tar -xf mpich-3.2.tar.gz -C .
```

Now we create an out-of-tree build location, configure, build, and install it

```bash
mkdir $STACK_SRC/mpich-3.2/gcc-build
cd $STACK_SRC/mpich-3.2/gcc-build

../configure --prefix=$PACKAGES_DIR/mpich-3.2 \
--enable-shared \
--enable-sharedlibs=gcc \
--enable-fast=03 \
--enable-debuginfo \
--enable-totalview \
--enable-two-level-namespace \
CC=gcc \
CXX=g++ \
FC=gfortran \
F77=gfortran \
F90='' \
CFLAGS='' \
CXXFLAGS='' \
FFLAGS='' \
FCFLAGS='' \
F90FLAGS='' \
F77FLAGS=''

make -j # (where # is the number of cores available)

make install
```

!alert! note
In order to utilize our newly built MPI wrapper, we need to set some variables:

```bash
export PATH=$PACKAGES_DIR/mpich-3.2/bin:$PATH
export CC=mpicc
export CXX=mpicxx
export FC=mpif90
export F90=mpif90
export C_INCLUDE_PATH=$PACKAGES_DIR/mpich-3.2/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=$PACKAGES_DIR/mpich-3.2/include:$CPLUS_INCLUDE_PATH
export FPATH=$PACKAGES_DIR/mpich-3.2/include:$FPATH
export MANPATH=$PACKAGES_DIR/mpich-3.2/share/man:$MANPATH
export LD_LIBRARY_PATH=$PACKAGES_DIR/mpich-3.2/lib:$LD_LIBRARY_PATH
```
!alert-end!

## PETSc

Download PETSc 3.6.4

```bash
cd $STACK_SRC
curl -L -O http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-3.8.3.tar.gz
tar -xf petsc-3.8.3.tar.gz -C .
```

Now we configure, build, and install it

```bash
cd $STACK_SRC/petsc-3.8.3

./configure \
--prefix=$PACKAGES_DIR/petsc-3.8.3 \
--download-hypre=1 \
--with-ssl=0 \
--with-debugging=no \
--with-pic=1 \
--with-shared-libraries=1 \
--with-cc=mpicc \
--with-cxx=mpicxx \
--with-fc=mpif90 \
--download-fblaslapack=1 \
--download-metis=1 \
--download-parmetis=1 \
--download-superlu_dist=1 \
--download-mumps=1 \
--download-scalapack=1 \
--CC=mpicc --CXX=mpicxx --FC=mpif90 --F77=mpif77 --F90=mpif90 \
--CFLAGS='-fPIC -fopenmp' \
--CXXFLAGS='-fPIC -fopenmp' \
--FFLAGS='-fPIC -fopenmp' \
--FCFLAGS='-fPIC -fopenmp' \
--F90FLAGS='-fPIC -fopenmp' \
--F77FLAGS='-fPIC -fopenmp' \
--LDFLAGS='-L$PACKAGES_DIR/gcc-7.3.0/lib64'
```

Once configure is done, we build PETSc

```bash
make PETSC_DIR=$STACK_SRC/petsc-3.8.3 PETSC_ARCH=arch-linux2-c-opt all
```

Everything good so far? PETSc should be asking to run more make commands

```bash
make PETSC_DIR=$STACK_SRC/petsc-3.8.3 PETSC_ARCH=arch-linux2-c-opt install
```

And now after the install, we can run some built-in tests

```bash
make PETSC_DIR=$PACKAGES_DIR/petsc-3.8.3 PETSC_ARCH="" test
```

Running the tests should produce some output like the following:

```bash
[moose@centos-7 petsc-3.6.3]$ make PETSC_DIR=$PACKAGES_DIR/petsc-3.8.3 PETSC_ARCH="" test
Running test examples to verify correct installation
Using PETSC_DIR=/opt/MOOSE/petsc-3.8.3 and PETSC_ARCH=
C/C++ example src/snes/examples/tutorials/ex19 run successfully with 1 MPI process
C/C++ example src/snes/examples/tutorials/ex19 run successfully with 2 MPI processes
Fortran example src/snes/examples/tutorials/ex5f run successfully with 1 MPI process
Completed test examples
=========================================
```

## Change Ownership

We are done building libraries, so lets chown up the target directory appropriately

```bash
sudo chown -R root:root $PACKAGES_DIR
```

This is more of a formality step, so any potential user of your newly built compiler stack does not
see everything owned by a non-root user.

## bash_profile

Now that PETSc has been successfully installed and tested, its time to wrap all these environment
variables up, and throw them in a bash shell profile somewhere.

Append the following contents into moose-environment.sh:

```bash
#!/bin/bash
### MOOSE Environment Profile

### 7-13-2016

###
### GCC 7.3.0

### MPICH 3.2

### PETSc 3.8.3

export PACKAGES_DIR=/opt/moose

export PATH=$PACKAGES_DIR/gcc-7.3.0/bin:$PACKAGES_DIR/mpich-3.2/bin:$PATH

export LD_LIBRARY_PATH=$PACKAGES_DIR/gcc-7.3.0/lib64:$PACKAGES_DIR/gcc-7.3.0/lib:$PACKAGES_DIR/gcc-7.3.0/lib/gcc/x86_64-unknown-linux-gnu/7.3.0:$PACKAGES_DIR/gcc-7.3.0/libexec/gcc/x86_64-unknown-linux-gnu/7.3.0:$PACKAGES_DIR/mpich-3.2/lib:$LD_LIBRARY_PATH

export C_INCLUDE_PATH=$PACKAGES_DIR/mpich-3.2/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=$PACKAGES_DIR/mpich-3.2/include:$CPLUS_INCLUDE_PATH
export FPATH=$PACKAGES_DIR/mpich-3.2/include:$FPATH
export MANPATH=$PACKAGES_DIR/mpich-3.2/share/man:$MANPATH

export PETSC_DIR=$PACKAGES_DIR/petsc-3.8.3

export CC=mpicc
export CXX=mpicxx
export FC=mpif90
export F90=mpif90
```

Thats it! Now you can either source this file manually each time you need to work on a MOOSE based
application:

```bash
source /path/to/moose-environment.sh
```

Or you can permanently have it loaded each time you open a terminal by adding the above `source`
command in your ~/.bash_profile (or ~/.bashrc which ever your system uses).

!include getting_started/minimum_requirements.md

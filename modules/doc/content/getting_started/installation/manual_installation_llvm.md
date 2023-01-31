# Manual Installation LLVM/MPICH

!alert! note title=Are you sure?
Getting a proper LLVM/MPICH compiler built is substantially more difficult than our [GCC/MPICH](getting_started/installation/manual_installation_gcc.md) combination. We need to build LLVM using a *modern* GCC compiler (not just a minimal C++17 compliant GCC compiler). Also, the higher the version of GCC, does not always guarantee success.

Furthermore, your millage will vary depending on the operating system. Building LLVM is a finicky process. If you are unsure as to why you are building an LLVM compiler, it might be best to build the [GCC/MPICH](getting_started/installation/manual_installation_gcc.md) compiler stack instead.
!alert-end!

!include sqa/minimum_requirements.md

## Prerequisites

- Cmake 3.4 or greater will be needed for building LLVM and some of the optional packages distributed with PETSc that MOOSE requires. Unless your system is very old, one should be able to use their system's package manager (apt-get, yum, zypper, etc) to install a compatible version of Cmake. For older systems, you will need to obtain cmake source from http://www.cmake.org, and build it appropriately for your system.

!include manual_prereqs.md

!include manual_environment.md

!include manual_gcc.md

!include manual_llvm.md

!alert! note
In order to utilize our newly built LLVM-Clang compiler, we need to export some variables:

!package! code
export CC=clang
export CXX=clang++
export PATH=$PACKAGES_DIR/llvm-__LLVM__/bin:$PATH
export LD_LIBRARY_PATH=$PACKAGES_DIR/llvm-__LLVM__/lib:$LD_LIBRARY_PATH
!package-end!

!alert-end!

!include manual_mpich_llvm.md

!include getting_started/installation/manual_miniconda.md

## bash_profile

Now that everything has been installed, its time to wrap all these environment variables up, and throw them in a bash shell profile somewhere.

Append the following contents into a new file called `moose-environment.sh`:

!package! code
#!/bin/bash
### MOOSE Environment Profile
# GCC __GCC__
# LLVM __LLVM__
# MPICH __MPICH__

export PACKAGES_DIR=<what ever you exported initially during the Environment setup>

export PATH=$PACKAGES_DIR/llvm-__LLVM__/bin:$PACKAGES_DIR/gcc-__GCC__/bin:$PACKAGES_DIR/mpich-__MPICH__/bin:$PACKAGES_DIR/miniconda/bin:$PATH
export LD_LIBRARY_PATH=$PACKAGES_DIR/llvm-__LLVM__/lib:$PACKAGES_DIR/gcc-__GCC__/lib64:$PACKAGES_DIR/gcc-__GCC__/lib:$PACKAGES_DIR/gcc-__GCC__/lib/gcc/x86_64-pc-linux-gnu/__GCC__:$PACKAGES_DIR/gcc-__GCC__/libexec/gcc/x86_64-pc-linux-gnu/__GCC__:$PACKAGES_DIR/mpich-__MPICH__/lib:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=$PACKAGES_DIR/mpich-__MPICH__/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=$PACKAGES_DIR/mpich-__MPICH__/include:$CPLUS_INCLUDE_PATH
export FPATH=$PACKAGES_DIR/mpich-__MPICH__/include:$FPATH
export MANPATH=$PACKAGES_DIR/mpich-__MPICH__/share/man:$MANPATH
export CC=mpicc
export CXX=mpicxx
export FC=mpif90
export F90=mpif90
!package-end!

Thats it! Now you can either source this file manually each time you need to work on a MOOSE based
application:

```bash
source /path/to/moose-environment.sh
```

Or you can permanently have it loaded each time you open a terminal by adding the above `source`
command in your ~/.bash_profile (or ~/.bashrc which ever your system uses).


## Compiler Stack Finished

With the compiler stack ready, you can proceed to [Obtaining and Building MOOSE](getting_started/installation/install_moose.md).

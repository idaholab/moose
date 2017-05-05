# Manual Installation
!include docs/content/getting_started/minimum_requirements.md

---
## Pre-Reqs
* Cmake 3.4 or greater will be needed when building Clang. We will also need Cmake for building PETSc. Unless your system is very old, one should be able to use their system's package manager (apt-get, yum, zypper, etc) to install a compatible version of Cmake. For older systems, you will need to obtain cmake source from http://www.cmake.org, and build it appropriately for your system.

* Python 2.7 is required to build Clang 3.7.0. Please use your package manager or other documentation on installing Python 2.7 side-by-side or otherwise known as `altinstall`, so as not to overwrite your current older 'possibly needed' Python implementation. An easy set of instructions for doing this can be found all over the internets. Personally, I have used the following with success: [http://tecadmin.net/install-python-2-7-on-centos-rhel](http://tecadmin.net/install-python-2-7-on-centos-rhel)

* A sane environment. This means having a clean, nothing but the bare minimum as far as available libraries go in your running environment. No additional LD_LIBRARY_PATHs, or other extra PATHs set. No strange UMASK settings. No odd aliases. It might even be best, to create a separate account, strictly for the use of these instructions. I have created an account called 'moose', and will assume you have done the same.

* Sane Environment
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
!!! Important
    What ever terminal window you were in, when you performed the above exports and umask commands, you _MUST_ use that same window, for the remainder of these instructions. If this window is closed, or the machine is rebooted, it will be necessary to perform the above commands again, before continuing any step. You will also _need_ to perform any exports in any previous steps you continued from.


And now we create our target installation location. We will also `chown` the location to our own user id for now. This will allow us to perform all the `make install` commands with out the need of sudo, which can complicate things.
```bash
mkdir -p $STACK_SRC
sudo mkdir -p $PACKAGES_DIR
sudo chown -R moose $PACKAGES_DIR
```

---
## GCC
We need a modern C++11 capable compiler. Our minimum requirements will be:  GCC 4.8.4, Clang 3.4.0, Intel20130607. This document will focus on building a Clang 3.7.0 compiler stack. Because we are focusing on building a Clang compiler (Clangs ability to compile C++11 code is substantially quicker than GCC), this document's GCC version will be 5.4.0 (the minimum GCC version to build Clang 3.7.0 is GCC 5.2.x). If your available system GCC compiler is at, or greater than 5.2.0, you can effectively skip this step.

What version of GCC do we have?
```bash
[]$ gcc --version
gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-4)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```
Looks like I need to build a newer version of GCC. Lets get started!

```bash
cd $STACK_SRC
curl -L -O http://mirrors.concertpass.com/gcc/releases/gcc-5.4.0/gcc-5.4.0.tar.gz
tar -xf gcc-5.4.0.tar.gz -C .
```

Obtain GCC pre-reqs:
```bash
cd $STACK_SRC/gcc-5.4.0
./contrib/download_prerequisites
```

Configure, build and install GCC:
```bash
mkdir $STACK_SRC/gcc-build
cd $STACK_SRC/gcc-build

../gcc-5.4.0/configure --prefix=$PACKAGES_DIR/gcc-5.4.0 \
--enable-multilib \
--enable-languages=c,c++,fortran \
--enable-lto

make -j #   (where # is the number of cores available)

make install
```
Any errors during configure/make will need to be investigated on your own. Every operating system I have come across has its own nuances of getting stuff built. Normally any issues are going to be solved by installing the necessary development libraries using your system package manager (apt-get, yum, zypper, etc). Hint: I would search the internet for 'how to build GCC 5.4.0 on (insert the name/version of your operating system here)'

!!! Important
    In order to utilize our newly built GCC 5.4.0 compiler, we need to set some variables:
    <pre>
    export PATH=$PACKAGES_DIR/gcc-5.4.0/bin:$PATH
    export LD_LIBRARY_PATH=$PACKAGES_DIR/gcc-5.4.0/lib64:$PACKAGES_DIR/gcc-5.4.0/lib:$PACKAGES_DIR/gcc-5.4.0/lib/gcc/x86_64-unknown-linux-gnu/5.4.0:$PACKAGES_DIR/gcc-5.4.0/libexec/gcc/x86_64-unknown-linux-gnu/5.4.0:$LD_LIBRARY_PATH
    </pre>

---
## Clang
We will clone all the necessary repositories involved with building LLVM/Clang from source:
```bash
mkdir -p $STACK_SRC/llvm-src
cd $STACK_SRC/llvm-src
git clone https://github.com/llvm-mirror/llvm.git
git clone https://github.com/llvm-mirror/clang.git $STACK_SRC/llvm-src/llvm/tools/clang
git clone https://github.com/llvm-mirror/compiler-rt.git $STACK_SRC/llvm-src/llvm/projects/compiler-rt
git clone https://github.com/llvm-mirror/libcxx.git $STACK_SRC/llvm-src/llvm/projects/libcxx
git clone https://github.com/llvm-mirror/libcxxabi.git $STACK_SRC/llvm-src/llvm/projects/libcxxabi
git clone https://github.com/llvm-mirror/openmp.git $STACK_SRC/llvm-src/llvm/projects/openmp
git clone https://github.com/llvm-mirror/clang-tools-extra.git $STACK_SRC/llvm-src/llvm/tools/clang/tools/extra

cd $STACK_SRC/llvm-src/llvm
git checkout release_37
$STACK_SRC/llvm-src/llvm/tools/clang
git checkout release_37
cd $STACK_SRC/llvm-src/llvm/projects/compiler-rt
git checkout release_37
cd $STACK_SRC/llvm-src/llvm/projects/libcxx
git checkout release_37
cd $STACK_SRC/llvm-src/llvm/projects/libcxxabi
git checkout release_37
cd $STACK_SRC/llvm-src/llvm/projects/openmp
git checkout release_37
cd $STACK_SRC/llvm-src/llvm/tools/clang/tools/extra
git checkout release_37
```

And now we configure, build, and install Clang:
```bash
mkdir -p $STACK_SRC/llvm-src/build
cd $STACK_SRC/llvm-src/build
cmake -DCMAKE_INSTALL_RPATH:STRING=$PACKAGES_DIR/llvm-3.7.0/lib \
-DCMAKE_CXX_LINK_FLAGS="-L$PACKAGES_DIR/gcc-5.4.0/lib64 -Wl,-rpath,$PACKAGES_DIR/gcc-5.4.0/lib64" \
-DGCC_INSTALL_PREFIX=$PACKAGES_DIR/gcc-5.4.0 \
-DCMAKE_CXX_COMPILER=$PACKAGES_DIR/gcc-5.4.0/bin/g++ \
-DCMAKE_C_COMPILER=$PACKAGES_DIR/gcc-5.4.0/bin/gcc \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX=$PACKAGES_DIR/llvm-3.7.0 \
-DCMAKE_INSTALL_NAME_DIR:STRING=$PACKAGES_DIR/llvm-3.7.0/lib \
-DCMAKE_MACOSX_RPATH:BOOL=OFF \
-DPYTHON_EXECUTABLE=`which python2.7` \
-DLLVM_TARGETS_TO_BUILD="X86" \
-G 'Unix Makefiles' ../llvm

make -j # (where # is the number of cores available)

make install
```
!!! Important
    In order to utilize our newly built LLVM-Clang compiler, we need to set some variables:
    <pre>
    export PATH=$PACKAGES_DIR/llvm-3.7.0/bin:$PATH
    export LD_LIBRARY_PATH=$PACKAGES_DIR/llvm-3.7.0/lib:$LD_LIBRARY_PATH
    </pre>


!!! Info
    Because we have a revision controlled checkout of LLVM, it is easy to switch between different releases if one has need of it. For example, what is listed here is 'release_37', but on my CentOS 7 test machine for which I have been using in creating these instructions, I have found no release would work, except 'master'. Which at the time of this writing, was hash: `695eea88e991443e69a980fed224ad1d9abf631e` (in case anyone finds that useful).


---
## MPICH
Download MPICH 3.2
```bash
cd $STACK_SRC
curl -L -O http://www.mpich.org/static/downloads/3.2/mpich-3.2.tar.gz
tar -xf mpich-3.2.tar.gz -C .
```

Now we create an out-of-tree build location, configure, build, and install it

```bash
mkdir $STACK_SRC/mpich-3.2/clang-build
cd $STACK_SRC/mpich-3.2/clang-build

../configure --prefix=$PACKAGES_DIR/mpich-3.2 \
--enable-shared \
--enable-sharedlibs=clang \
--enable-fast=03 \
--enable-debuginfo \
--enable-totalview \
--enable-two-level-namespace \
CC=clang \
CXX=clang++ \
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

!!! important
    In order to utilize our newly built MPI wrapper, we need to set some variables:
    <pre>
    export PATH=$PACKAGES_DIR/mpich-3.2/bin:$PATH
    export CC=mpicc
    export CXX=mpicxx
    export FC=mpif90
    export F90=mpif90
    export C_INCLUDE_PATH=$PACKAGES_DIR/mpich-3.2/include:$C_INCLUDE_PATH
    export CPLUS_INCLUDE_PATH=$PACKAGES_DIR/mpich-3.2/include:$CPLUS_INCLUDE_PATH
    export FPATH=$PACKAGES_DIR/mpich-3.2/include:$FPATH
    export MANPATH=$PACKAGES_DIR/mpich-3.2/share/man:$MANPATH
    export LD_LIBRARY_PATH=$PACKAGES_DIR/mpich-3.2/lib:$PACKAGES_DIR/mpich-3.2/lib/openmpi:$LD_LIBRARY_PATH
    </pre>

---
## PETSc
Download PETSc 3.6.4
```bash
cd $STACK_SRC
curl -L -O http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-3.6.4.tar.gz
tar -xf petsc-3.6.4.tar.gz -C .
```

Now we configure, build, and install it
```bash
cd $STACK_SRC/petsc-3.6.4

./configure \
--prefix=$PACKAGES_DIR/petsc-3.6.4 \
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
--LDFLAGS='-L$PACKAGES_DIR/gcc-5.4.0/lib64'
```

Once configure is done, we build PETSc
```bash
make PETSC_DIR=$STACK_SRC/petsc-3.6.4 PETSC_ARCH=arch-linux2-c-opt all
```
Everything good so far? PETSc should be asking to run more make commands
```bash
make PETSC_DIR=$STACK_SRC/petsc-3.6.4 PETSC_ARCH=arch-linux2-c-opt install
```
And now after the install, we can run some built-in tests
```bash
make PETSC_DIR=$PACKAGES_DIR/petsc-3.6.4 PETSC_ARCH="" test
```
Running the tests should produce some output like the following:

```bash
[moose@centos-7 petsc-3.6.3]$ make PETSC_DIR=$PACKAGES_DIR/petsc-3.6.4 PETSC_ARCH="" test
Running test examples to verify correct installation
Using PETSC_DIR=/opt/MOOSE/petsc-3.6.3 and PETSC_ARCH=
C/C++ example src/snes/examples/tutorials/ex19 run successfully with 1 MPI process
C/C++ example src/snes/examples/tutorials/ex19 run successfully with 2 MPI processes
Fortran example src/snes/examples/tutorials/ex5f run successfully with 1 MPI process
Completed test examples
=========================================
```

---
## Change Ownership
We are done building libraries, so lets chown up the target directory appropriately
```bash
sudo chown -R root:root $PACKAGES_DIR
```
This is more of a formality step, so any potential user of your newly built compiler stack does not see everything owned by a non-root user.

---
## bash_profile
Now that PETSc has been successfully installed and tested, its time to wrap all these environment variables up, and throw them in a bash shell profile somewhere.

Append the following contents into moose-environment.sh:

```bash
#!/bin/bash
### MOOSE Environment Profile
### 7-13-2016
###
### GCC 5.4.0
### LLVM-Clang 3.7.0
### MPICH 3.2
### PETSc 3.6.4

export PACKAGES_DIR=/opt/moose

export PATH=$PACKAGES_DIR/llvm-3.7.0/bin:$PACKAGES_DIR/gcc-5.4.0/bin:$PACKAGES_DIR/mpich-3.2/bin:$PATH

export LD_LIBRARY_PATH=$PACKAGES_DIR/llvm-3.7.0/lib:$PACKAGES_DIR/gcc-5.4.0/lib64:$PACKAGES_DIR/gcc-5.4.0/lib:$PACKAGES_DIR/gcc-5.4.0/lib/gcc/x86_64-unknown-linux-gnu/5.4.0:$PACKAGES_DIR/gcc-5.4.0/libexec/gcc/x86_64-unknown-linux-gnu/5.4.0:$PACKAGES_DIR/mpich-3.2/lib:$LD_LIBRARY_PATH

export C_INCLUDE_PATH=$PACKAGES_DIR/mpich-3.2/include:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=$PACKAGES_DIR/mpich-3.2/include:$CPLUS_INCLUDE_PATH
export FPATH=$PACKAGES_DIR/mpich-3.2/include:$FPATH
export MANPATH=$PACKAGES_DIR/mpich-3.2/share/man:$MANPATH

export PETSC_DIR=$PACKAGES_DIR/petsc-3.6.4

export CC=mpicc
export CXX=mpicxx
export FC=mpif90
export F90=mpif90
```

Thats it! Now you can either source this file manually each time you need to work on a MOOSE based application:
```bash
source /path/to/moose-environment.sh
```
Or you can permanently have it loaded each time you open a terminal by adding the above `source` command in your ~/.bash_profile (or ~/.bashrc which ever your system uses).

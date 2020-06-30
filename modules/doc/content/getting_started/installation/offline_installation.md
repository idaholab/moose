# Offline Installation

If you are using a machine (or HPC cluster) with no access to the internet, the following instructions will help you create an environment suitable for MOOSE-based development.

Please be certain, that the machine in which you intend to perform the actual work (which this document will refer to as the 'target' machine), meets our minimum requirements:

!include sqa/minimum_requirements.md

## Dependency List

Ultimately, we will need to build the following stack, to create an environment suitable for MOOSE-based development on the target machine:

- GCC, Or LLVM +and+ GCC
- MPI Wrapper
- PETSc, libMesh, and MOOSE

## Prerequisites

Before you begin, know that you will first need to be operating on a machine with internet access.

If you are on a Windows machine, please install [Ubuntu 20.04](https://www.microsoft.com/en-us/p/ubuntu-2004-lts/9n6svws3rx71) from the Microsoft store, simply to have access to a Linux terminal. Once inside the terminal, install 'git' and 'curl', as that will be needed to clone the repositories involved in later steps (`sudo apt-get install git curl`).

## GCC

It is highly likely the target machine in which you will be operating on will already contain an acceptable version of GCC. Please contact your system administrator for instructions on how to use that systems compiler. If not, you will need to head on over to GNU's [GCC](https://gcc.gnu.org/) site, and follow their instructions on building a GCC compiler at or beyond the above listed minimum version.

## LLVM (optional)

While arguably a [better compiler](https://opensource.apple.com/source/clang/clang-23/clang/tools/clang/www/comparison.html) over traditional GCC, it is more difficult to build. Nevertheless, if you wish to make the attempt, head on over to [llvm.org](http://llvm.org/), to begin.

!alert note
If you choose LLVM, know that you will also need to build (or use) a GCC toolchain! This is due to the lack of a Fortran compiler included with LLVM.

## MPI Wrapper

If the target machine is an HPC system, it is likely your system already has an MPI wrapper (and thus, a GCC or LLVM compiler). Please contact your system administrator, and ask them how to appropriately make use of that cluster's MPI wrapper. If, by chance you will have to build your own, you have some choices to make; [OpenMPI](https://www.open-mpi.org/), [MVAPICH](https://mvapich.cse.ohio-state.edu/), and [MPICH](https://www.mpich.org/), all work well. You will want to head on over to one of those product sites, and follow their instructions to build yourself an MPI wrapper (using the GCC or LLVM compiler you choose above).

!alert note
MVAPICH is regarded as the better wrapper for HPC clusters. However, it is also the most difficult to build properly for that clusters specific hardware. While you may be able to *build* it easily enough, you may not have built it *properly* to optimally make use of said cluster's hardware.

## Prepare PETSc, libMesh, and MOOSE for copying

These three libraries can all be obtained from one repository (the moose repository). First, create a top-level directory named 'offline', which will ultimately contain everything we need to transfer over to your target machine.

```bash
mkdir -p ~/offline/downloads
```

Next, enter the offline directory, and perform the cloning operation to obtain all three products:

```bash
cd ~/offline
git clone https://github.com/idaholab/moose.git
cd moose
git checkout master
git submodule update --init
git submodule foreach --recursive git submodule update --init
```

With PETSc cloned as part of the group obtained above, we can use a configure option in PETSc, to obtain a list of contributions we will need to download manually (`--with-package-download-dir`):

```bash
cd ~/offline/moose/petsc
./configure \
 --download-mumps=1 \
 --download-hypre=1 \
 --download-slepc=1 \
 --download-metis=1 \
 --download-ptscotch=1 \
 --download-parmetis=1 \
 --download-scalapack=1 \
 --download-fblaslapack=1 \
 --download-superlu_dist=1 \
 --with-packages-download-dir=~/offline/downloads
```

As an example, the above command, should return something like this:

```pre
===============================================================================
             Configuring PETSc to compile on your system
===============================================================================
Download the following packages to /home/you/offline/downloads

fblaslapack ['git://https://bitbucket.org/petsc/pkg-fblaslapack', 'https://bitbucket.org/petsc/pkg-fblaslapack/get/v3.4.2-p2.tar.gz']
hypre ['git://https://github.com/hypre-space/hypre', 'https://github.com/hypre-space/hypre/archive/93baaa8c9.tar.gz']
metis ['git://https://bitbucket.org/petsc/pkg-metis.git', 'https://bitbucket.org/petsc/pkg-metis/get/v5.1.0-p8.tar.gz']
parmetis ['git://https://bitbucket.org/petsc/pkg-parmetis.git', 'https://bitbucket.org/petsc/pkg-parmetis/get/v4.0.3-p6.tar.gz']
ptscotch ['git:https://gitlab.inria.fr/scotch/scotch', 'http://ftp.mcs.anl.gov/pub/petsc/externalpackages/scotch-v6.0.8.tar.gz']
mumps ['git://https://bitbucket.org/petsc/pkg-mumps.git', 'https://bitbucket.org/petsc/pkg-mumps/get/v5.2.1-p2.tar.gz']
scalapack ['git://https://bitbucket.org/petsc/pkg-scalapack', 'https://bitbucket.org/petsc/pkg-scalapack/get/v2.0.2-p2.tar.gz']
superlu_dist ['git://https://github.com/xiaoyeli/superlu_dist', 'https://github.com/xiaoyeli/superlu_dist/archive/v6.2.0.tar.gz']
slepc ['git://https://gitlab.com/slepc/slepc.git', 'https://gitlab.com/slepc/slepc/-/archive/bda551b/slepc-bda551b.tar.gz']
```

Your job, will be to parse through the above jargon, and download these packages to `~/offline/downloads`. Be certain to preserve the file name. PETSc is listing several means for which you can obtain these contributions (using either git or traditional web link). For the purpose of simplicity, we will use the traditional web method.

+Example ONLY, as file names may be deprecated!:+

```bash
cd ~/offline/downloads
curl -L -O https://bitbucket.org/petsc/pkg-fblaslapack/get/v3.4.2-p2.tar.gz
curl -L -O https://github.com/hypre-space/hypre/archive/93baaa8c9.tar.gz
curl -L -O https://bitbucket.org/petsc/pkg-metis/get/v5.1.0-p8.tar.gz
curl -L -O https://bitbucket.org/petsc/pkg-parmetis/get/v4.0.3-p6.tar.gz
curl -L -O http://ftp.mcs.anl.gov/pub/petsc/externalpackages/scotch-v6.0.8.tar.gz
curl -L -O https://bitbucket.org/petsc/pkg-mumps/get/v5.2.1-p2.tar.gz
curl -L -O https://bitbucket.org/petsc/pkg-scalapack/get/v2.0.2-p2.tar.gz
curl -L -O https://github.com/xiaoyeli/superlu_dist/archive/v6.2.0.tar.gz
curl -L -O https://gitlab.com/slepc/slepc/-/archive/bda551b/slepc-bda551b.tar.gz
```

At this point, everything necessary should be downloaded and available in the directory hierarchy at `~/offline`. At this time, you may copy this directory to your offline-no-internet access machine's home directory. At the time of this writing, tallying up the disc space used equates to approximately ~2GB. Depending on your internet connection, you may want to compress the entire ~/offline directory instead (saves about 500Mb):

```bash
cd ~/
tar -pzcf offline.tar.gz offline
```

!alert note
If operating on a Macintosh machine, creating a tarball to be extracted on a Linux machine produces warnings. They are warnings only, and can be safely ignored.

If copying the tarball over, you can extract it with:

```bash
tar -xf offline.tar.gz -C ~/
```

## Build PETSc, libMesh, and MOOSE

With the `~/offline` directory available in your target machine's home directory, we can now build PETSc, libMesh, and MOOSE, using your MPI Wrapper/Compiler established in earlier steps.

### PETSc

Configure, build, and install PETSc to:`$HOME/libs/petsc`

First, you should set your environment to make use of the compiler and MPI wrapper you established in earlier steps. On HPC machines, this normally involves loading modules. On a workstation, this may mean adjusting your PATH environment variable.

Verify an MPI wrapper is available:

```bash
which mpicc mpicxx mpif90 mpif77
```

The `which` command above should return the paths to your MPI wrappers established in earlier steps. If it returns nothing, or fewer paths than the 4 we were asking for, something is wrong. You will need to figure out how to enable your MPI wrapper before proceeding.

With your compilers ready for use, we can now build PETSc:

```bash
cd ~/offline/moose/petsc
./configure \
 --download-mumps=1 \
 --download-hypre=1 \
 --download-slepc=1 \
 --download-metis=1 \
 --download-ptscotch=1 \
 --download-parmetis=1 \
 --download-scalapack=1 \
 --download-fblaslapack=1 \
 --download-superlu_dist=1 \
 --with-packages-download-dir=~/offline/downloads \
 --with-mpi=1 \
 --with-debugging=no \
 --with-cxx-dialect=C++11 \
 --with-fortran-bindings=0 \
 --with-shared-libraries=1 \
 --prefix=$HOME/libs/petsc && make && make install
```

Unfortunately, any errors incurred during the above step is going to be beyond the scope of this document. Most likely, an error will be related to a missing library by one of the myriad contributions we are asking to build PETSc with. Please submit a detailed log of the error, to our [moose-users mailing list](https://groups.google.com/forum/#!forum/moose-users). But do be prepared to be asked to contact your system administrator; Errors of this nature normally require admin rights to fulfill the dependency.

Proceed only if PETSc completed successfully.

### libMesh

Configure, build, and install libMesh to: `$HOME/libs/libmesh`

First, we need to tell libMesh where PETSc is installed, and instruct libMesh to specifically use MPI:

```bash
export PETSC_DIR=$HOME/libs/petsc
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77
```

Now we can configure, build, and install libMesh:

```bash
cd ~/offline/moose/libmesh
./configure \
 --enable-silent-rules \
 --enable-unique-id \
 --disable-warnings \
 --enable-glibcxx-debugging \
 --with-thread-model=openmp \
 --disable-maintainer-mode \
 --enable-petsc-hypre-required \
 --enable-metaphysicl-required \
 --with-methods="opt dbg devel" \
 --prefix=$HOME/libs/libmesh && make -j 6 && make install
```

Unfortunately, any errors incurred during these steps is going to be beyond the scope of this document. Please submit a detailed log of the error, to our [moose-users mailing list](https://groups.google.com/forum/#!forum/moose-users).

Proceed only if libMesh completed successfully.

### MOOSE

First, we need to tell MOOSE  where libMesh is:

```bash
export LIBMESH_DIR=$HOME/libs/libmesh
```

Now we can build MOOSE:

```bash
export MOOSE_DIR=~/offline/moose
cd $MOOSE_DIR/test
make -j 6
```

Again, any errors incurred during this step, is going to be beyond the scope of this document. Please submit a detailed log of the error, to our [moose-users mailing list](https://groups.google.com/forum/#!forum/moose-users).

## Your Application

With all the libraries built, you are now able to build your application. To recap, there were several environment variables we previously set, which you will need to always set, when you perform any MOOSE-based development. Here are those environment variables again (setting them multiple times is harmless):

```bash
export PATH=/some/path/to/MPI/bin:/some/path/to/GCC/bin:$PATH <--EXAMPLE, CHANGE ME
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77
export PETSC_DIR=$HOME/libs/petsc
export LIBMESH_DIR=$HOME/libs/libmesh
export MOOSE_DIR=$HOME/offline/moose
```

You will need to perform the above, each and every time you log into the target machine and wish to perform development on your application. To that end, creating a special profile you can source in one command is recommended.

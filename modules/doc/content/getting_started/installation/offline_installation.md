# Offline Installation

If you are using a machine (or HPC cluster) with no access to the internet, the following
instructions will help you create an environment suitable for MOOSE-based development.

Please be certain, that the machine in which you intend to perform the actual work (which this
document will refer to as the 'target' machine), meets the following requirements:

!include sqa/minimum_requirements.md

# Prerequisites

#### Air-Gapped HPC Clusters

If the target machine is an HPC cluster, it is likely your cluster already has an MPI wrapper (and
thus a GCC or LLVM compiler). Please contact your system administrator and ask them how to
appropriately make use of your cluster's MPI wrapper and compiler. Please note, Intel compilers are
not supported.

#### Personal Workstation

If the target machine is your personal workstation, that machine must sufficiently achieve the
following:

!include installation/manual_prereqs.md

In addition, the target machine will need an MPI wrapper. We recommend one of the following:

- [MPICH](https://www.mpich.org/)

- [OpenMPI](https://www.open-mpi.org/)

!alert note title=Personal Air-Gapped Workstation
Procuring the above on a personal air-gapped workstation will need to be your responsibility. We
cannot instruct a means for bypassing such security.

## Prepare Directory

PETSc, libMesh and MOOSE can all be obtained from one repository (the moose repository). First,
create a top-level directory named 'offline', which will ultimately contain everything we need to
transfer over to your target machine.

```bash
mkdir -p ~/offline/downloads
```

Next, enter the offline directory, and perform the cloning operation to obtain all three libraries:

```bash
cd ~/offline
git clone https://github.com/idaholab/moose.git
cd moose
git checkout master
git submodule update --init
git submodule foreach --recursive git submodule update --init
```

With PETSc cloned as part of the group obtained above, we can use a configure option in PETSc, to
obtain a list of contributions we will need to download manually (`--with-package-download-dir`):

```bash
cd ~/offline/moose

./scripts/update_and_rebuild_petsc.sh  --with-packages-download-dir=~/offline/downloads
```

As an example, the above command should return something like the following:

```pre
===============================================================================
             Configuring PETSc to compile on your system
===============================================================================
Download the following packages to /home/you/offline/downloads

fblaslapack ['git://https://bitbucket.org/petsc/pkg-fblaslapack', 'https://bitbucket.org/petsc/pkg-fblaslapack/get/v3.4.2-p3.tar.gz']
hypre ['git://https://github.com/hypre-space/hypre', 'https://github.com/hypre-space/hypre/archive/93baaa8c9.tar.gz']
metis ['git://https://bitbucket.org/petsc/pkg-metis.git', 'https://bitbucket.org/petsc/pkg-metis/get/v5.1.0-p8.tar.gz']
parmetis ['git://https://bitbucket.org/petsc/pkg-parmetis.git', 'https://bitbucket.org/petsc/pkg-parmetis/get/v4.0.3-p6.tar.gz']
ptscotch ['git://https://gitlab.inria.fr/scotch/scotch.git', 'https://gitlab.inria.fr/scotch/scotch/-/archive/v6.0.9/scotch-v6.0.9.tar.gz', 'http://ftp.mcs.anl.gov/pub/petsc/externalpackages/scotch-v6.0.9.tar.gz']
mumps ['git://https://bitbucket.org/petsc/pkg-mumps.git', 'https://bitbucket.org/petsc/pkg-mumps/get/v5.2.1-p2.tar.gz']
scalapack ['git://https://bitbucket.org/petsc/pkg-scalapack', 'https://bitbucket.org/petsc/pkg-scalapack/get/v2.1.0-p1.tar.gz']
superlu_dist ['git://https://github.com/xiaoyeli/superlu_dist', 'https://github.com/xiaoyeli/superlu_dist/archive/v6.3.0.tar.gz']
slepc ['git://https://gitlab.com/slepc/slepc.git', 'https://gitlab.com/slepc/slepc/-/archive/v3.13.3/slepc-v3.13.3.tar.gz']
```

Your job, will be to parse through the above jargon, and download these packages to
`~/offline/downloads`. Be certain to preserve the file name. PETSc is listing several means for
which you can obtain these contributions (using either git or traditional web link). For the purpose
of simplicity, we will use the traditional web method.

+Example ONLY, as file names may be deprecated!:+

```bash
cd ~/offline/downloads
curl -L -O https://bitbucket.org/petsc/pkg-fblaslapack/get/v3.4.2-p3.tar.gz
curl -L -O https://github.com/hypre-space/hypre/archive/93baaa8c9.tar.gz
curl -L -O https://bitbucket.org/petsc/pkg-metis/get/v5.1.0-p8.tar.gz
curl -L -O https://bitbucket.org/petsc/pkg-parmetis/get/v4.0.3-p6.tar.gz
curl -L -O http://ftp.mcs.anl.gov/pub/petsc/externalpackages/scotch-v6.0.9.tar.gz
curl -L -O https://bitbucket.org/petsc/pkg-mumps/get/v5.2.1-p2.tar.gz
curl -L -O https://bitbucket.org/petsc/pkg-scalapack/get/v2.1.0-p1.tar.gz
curl -L -O https://github.com/xiaoyeli/superlu_dist/archive/v6.3.0.tar.gz
curl -L -O https://gitlab.com/slepc/slepc/-/archive/v3.13.3/slepc-v3.13.3.tar.gz
```

At this point, everything necessary should be downloaded and available in the directory hierarchy at
`~/offline`. At this time, you may copy this directory to your offline-no-internet access machine's
home directory. At the time of this writing, tallying up the disc space used equates to
approximately ~2GB. Depending on your internet connection, you may want to compress the entire
~/offline directory instead (saves about 500Mb):

```bash
cd ~/
tar -pzcf offline.tar.gz offline
```

!alert note
If operating on a Macintosh machine, creating a tarball to be extracted on a Linux machine produces
warnings. They are warnings only, and can be safely ignored.

If copying the tarball over, you can extract it with:

```bash
tar -xf offline.tar.gz -C ~/
```

## Build Libraries

With the `~/offline` directory available in your target machine's home directory, we can now build
PETSc, libMesh, and MOOSE, using your MPI Wrapper/Compiler established in earlier steps.

#### PETSc

Configure, build, and install PETSc to:`$HOME/libs/petsc`

First, you should set your environment to make use of the compiler and MPI wrapper you established
in earlier steps. On HPC machines, this normally involves loading modules. On a workstation, this
may mean adjusting your PATH environment variable.

Verify an MPI wrapper is available:

```bash
which mpicc mpicxx mpif90 mpif77
```

The `which` command above should return the paths to your MPI wrappers established in earlier steps.
If it returns nothing, or fewer paths than the 4 we were asking for, something is wrong. You will
need to figure out how to enable your MPI wrapper before proceeding.

With your compilers ready for use, we can now build PETSc:

```bash
cd ~/offline/moose

./scripts/update_and_rebuild_petsc.sh --skip-submodule-update --with-packages-download-dir=~/offline/downloads --prefix=$HOME/libs/petsc
```

Unfortunately, any errors incurred during the above step is going to be beyond the scope of this
document. Most likely, an error will be related to a missing library by one of the myriad
contributions we are asking to build PETSc with. Please submit a detailed log of the error, to the
[MOOSE Discussion forum](https://github.com/idaholab/moose/discussions). But do be prepared to be
asked to contact your system administrator; Errors of this nature normally require admin rights to
fulfill the dependency.

Proceed only if PETSc completed successfully.

!alert! note
If you prefer to install PETSc in place (moose/petsc), then you need to take out `--prefix`

```bash
cd ~/offline/moose

unset PETSC_DIR PETSC_ARCH
./scripts/update_and_rebuild_petsc.sh --skip-submodule-update --with-packages-download-dir=~/offline/downloads
```

In this case, you need to `unset PETSC_DIR PETSC_ARCH` during libmesh compile.

!alert-end!

#### libMesh

Configure, build, and install libMesh to: `$HOME/libs/libmesh`

First, we need to tell libMesh where PETSc is installed, and instruct libMesh to specifically use
MPI:

```bash
export PETSC_DIR=$HOME/libs/petsc
export LIBMESH_DIR=$HOME/libs/libmesh
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77
```

!alert! note
If you selected to install PETSc in place earlier, you do not need to set `PETSC_DIR`. libMesh
will pick up PETSc from petsc submodule. You may do the following to unset `PETSC_DIR`:

```bash
unset PETSC_DIR PETSC_ARCH
```

!alert-end!

!alert! note
If you want to install libMesh in place, you do not need to set `LIBMESH_DIR`. libMesh
will be installed to inside of libMesh submodule. You may do the following to unset `LIBMESH_DIR`:

```bash
unset LIBMESH_DIR
```

!alert-end!


Now we can configure, build, and install libMesh:

```bash
cd ~/offline/moose
./scripts/update_and_rebuild_libmesh.sh --skip-submodule-update
```

Unfortunately, any errors incurred during these steps is going to be beyond the scope of this
document. Please submit a detailed log of the error, to the
[MOOSE Discussion forum](https://github.com/idaholab/moose/discussions).

Proceed only if libMesh completed successfully.

#### MOOSE

First, we need to tell MOOSE  where libMesh is:

```bash
export LIBMESH_DIR=$HOME/libs/libmesh
```

!alert! note
As mentioned earlier, if libMesh is installed in place, you do no need to set `LIBMESH_DIR`.
MOOSE will look for libMesh submodule.

```bash
unset LIBMESH_DIR
```

!alert-end!

Now we can build MOOSE:

```bash
export MOOSE_DIR=~/offline/moose
cd $MOOSE_DIR/test
make -j 6
```

Again, any errors incurred during this step, is going to be beyond the scope of this document.
Please submit a detailed log of the error, to the
[MOOSE Discussion forum](https://github.com/idaholab/moose/discussions).

## Your Application

With all the libraries built, you are now able to build your application. To recap, there were
several environment variables we previously set, which you will need to always set, when you perform
any MOOSE-based development. Here are those environment variables again (setting them multiple times
is harmless):

```bash
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77
export MOOSE_DIR=$HOME/offline/moose
```

You will need to perform the above, each and every time you log into the target machine and wish to
perform development on your application. To that end, creating a special profile you can source in
one command is recommended.

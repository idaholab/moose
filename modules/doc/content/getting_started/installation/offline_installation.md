# Offline Installation

If you are using a machine (or HPC cluster) with no access to the internet, the following
instructions will help you create an environment suitable for MOOSE-based development.

Please be certain, that the machine in which you intend to perform the actual work (which this
document will refer to as the 'target' machine), meets the following requirements:

## Prerequisites

!include sqa/minimum_requirements.md

#### Air-Gapped HPC Clusters

!style! style=margin-left:30px
If the target machine is an [!ac](HPC) cluster, it is likely your cluster already has a proper toolchain
stack available. Please contact your system administrator and ask them how to appropriately make use
of your cluster's MPI wrapper and compiler. Please note, Intel compilers are not supported.
!style-end!

#### Personal Workstation

!style! style=margin-left:30px
If the target machine is your personal workstation, that machine must sufficiently achieve the
following:
!style-end!

!include installation/manual_prereqs.md

!style! style=margin-left:30px
In addition, the target machine will need an MPI wrapper. We recommend one of the following
products:
!style-end!

- [MPICH](https://www.mpich.org/)
- [OpenMPI](https://www.open-mpi.org/)
- [MVAPICH](https://mvapich.cse.ohio-state.edu/)

!style! style=margin-left:30px
!alert! tip title=System MPI Wrapper
For +Linux+ users, the easiest way to install an MPI wrapper is via the same system package manager
tool used obtaining a developers environment. Examples:

```bash
apt install openmpi  # Ubuntu, Debian
dnf install mpich    # CentOS, Rocky, RHEL
zypper install mpich # OpenSUSE
```
!alert-end!

!alert note title=Personal Air-Gapped Workstation
Procuring the above on a workstation with no network access is beyond the scope of this document.
Please work with your system administrator on satisfying the above prerequisites.

!style-end!

## Prepare Directory

!style! halign=left
PETSc, libMesh and MOOSE can all be obtained from one repository (the moose repository). First,
create a top-level directory named 'offline', which will ultimately contain everything we need to
transfer over to your target machine.
!style-end!

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

!style! halign=left
With the `~/offline` directory available in your target machine's home directory, we can now build
PETSc, libMesh, and MOOSE, using your MPI Wrapper/Compiler established in earlier steps.
!style-end!

#### Verify MPI

First, verify an MPI wrapper is in fact available. Do not close this terminal window after running
the following commands. We need the following set for the remainder of these instructions:

```bash
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77
which $CC $CXX $FC $F77
```

The `which` command above should return paths to your MPI wrappers.

If the above command returns nothing, or fewer paths than the 4 we are asking for, something is
wrong. You need to STOP, and figure out how to enable your MPI wrapper before proceeding. If you are
operating on an HPC cluster, this normally involves loading an MPI module. If you are operating on
your personal workstation, you need to follow the instructions on which MPI product you decided to
install during the Prerequisites section.

#### Build PETSc and libMesh

Build PETSc and libMesh by instructing these libraries to use the offline directory you created:

!template load file=installation/build_petsc_and_libmesh.md.template PATH=~/offline PETSC_ARGS1=--skip-submodule-update PETSC_ARGS2=--with-packages-download-dir=~/offline/downloads LIBMESH_ARGS1=--skip-submodule-update

Unfortunately, any errors incurred during the above step is going to be beyond the scope of this
document. Most likely, an error will be related to a missing library by one of the myriad
contributions we are asking to build PETSc with. Please submit a detailed log of the error, to the
[MOOSE Discussion forum](https://github.com/idaholab/moose/discussions). But do be prepared to be
asked to contact your system administrator; Errors of this nature normally require admin rights to
fulfill the dependency.

#### Build and Test MOOSE

!template load file=installation/build_moose.md.template PATH=~/offline

Again, any errors incurred during this step, is going to be beyond the scope of this document.
Please submit a detailed log of the error, to the
[MOOSE Discussion forum](https://github.com/idaholab/moose/discussions).

!template load file=installation/test_moose.md.template PATH=~/offline

## Your Application

!style! halign=left
With MOOSE built and testing successfully, you should now be able to build your application. The
only prerequisites before doing so, is to configure your environment to make use of your MPI
wrapper, and to define where your built copy of MOOSE resides:
!style-end!

```bash
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77
export MOOSE_DIR=$HOME/offline/moose
```

You will need to export the above variables each time you open a new terminal window on your HPC
cluster or personal workstation, before you can perform any application development.

!include installation/start_up_profile.md

Now that you have a working MOOSE, and you know how to make your MPI wrapper available, proceed to
[building your own application](installation/offline_new_users.md).

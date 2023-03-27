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

In addition, the target machine will need an MPI wrapper. We recommend one of the following
products:

- [MPICH](https://www.mpich.org/)

- [OpenMPI](https://www.open-mpi.org/)

Please choose one of the above and follow their instructions on how to build, install, and use your
MPI wrapper of choice.

!alert tip title=Linux MPI Wrapper
If you are operating on a personal Linux workstation, the easiest way to obtain an MPI wrapper is
via the same tool used to obtain a developers environment in the Prerequisites section.

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

#### Verify MPI

First, verify an MPI wrapper is in fact available:

```bash
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77
which $CC $CXX $FC $F77
```

The `which` command above should return paths to your MPI wrappers.

If the above command returns nothing, or fewer paths than the 4 we were asking for, something is
wrong. You need to STOP, and figure out how to enable your MPI wrapper before proceeding. If you are
operating on an HPC cluster, this normally involves loading an MPI module. If you are operating on
your personal workstation, you need to follow the instructions on which ever MPI product you decided
to install during the Prerequisites section.

#### Build PETSc

Build PETSc using the following convenient script in addition to the supplied arguments:

```bash
cd ~/offline/moose
./scripts/update_and_rebuild_petsc.sh --skip-submodule-update \
  --with-packages-download-dir=~/offline/downloads
```

Unfortunately, any errors incurred during the above step is going to be beyond the scope of this
document. Most likely, an error will be related to a missing library by one of the myriad
contributions we are asking to build PETSc with. Please submit a detailed log of the error, to the
[MOOSE Discussion forum](https://github.com/idaholab/moose/discussions). But do be prepared to be
asked to contact your system administrator; Errors of this nature normally require admin rights to
fulfill the dependency.

Proceed only if PETSc completed successfully.

#### Build libMesh

Build libMesh using the following convenient script in addition to the supplied argument:

```bash
cd ~/offline/moose
./scripts/update_and_rebuild_libmesh.sh --skip-submodule-update
```

Unfortunately, any errors incurred during these steps is going to be beyond the scope of this
document. Please submit a detailed log of the error, to the
[MOOSE Discussion forum](https://github.com/idaholab/moose/discussions).

Proceed only if libMesh completed successfully.

#### Build MOOSE

With the support libraries built, you can now build MOOSE:

```bash
cd ~/offline/moose/test
make -j 6
```

Again, any errors incurred during this step, is going to be beyond the scope of this document.
Please submit a detailed log of the error, to the
[MOOSE Discussion forum](https://github.com/idaholab/moose/discussions).


#### Test MOOSE

With MOOSE built, test to see if it works in your environment:

```bash
cd ~/offline/moose/test
./run_tests -j 6
```

Due to the nature of an offline install, it is possible some tests will fail (missing common system
binaries like `rsync`, or other network related tools not normally present on air-gapped machines).
You will also see SKIPPED tests. Some tests are specific to Macintosh, or Linux only. Or some other
constraint your machine does not satisfy.

However, most tests should pass. If they don't, or you see `MAX FAILURES`, thats a problem! And it
needs to be addressed before continuing. Please supply a report of the actual failure (scroll up a
ways). For example the following does not help us (created with `./run_tests -i always_bad`):

```pre
Final Test Results:
--------------------------------------------------------------------------------
tests/test_harness.always_ok .................... FAILED (Application not found)
tests/test_harness.always_bad .................................. FAILED (CODE 1)
--------------------------------------------------------------------------------
Ran 2 tests in 0.2 seconds. Average test time 0.0 seconds, maximum test time 0.0 seconds.
0 passed, 0 skipped, 0 pending, 2 FAILED
```

You need to scroll up, and find/report the actual error:

```pre
tests/test_harness.always_ok: Working Directory: /Users/me/projects/moose/test/tests/test_harness
tests/test_harness.always_ok: Running command:
tests/test_harness.always_ok:
tests/test_harness.always_ok: ######################################################################
tests/test_harness.always_ok: Tester failed, reason: Application not found
tests/test_harness.always_ok:
tests/test_harness.always_ok .................... FAILED (Application not found)
tests/test_harness.always_bad: Working Directory: /Users/me/projects/moose/test/tests/test_harness
tests/test_harness.always_bad: Running command: false
tests/test_harness.always_bad:
tests/test_harness.always_bad: #####################################################################
tests/test_harness.always_bad: Tester failed, reason: CODE 1
tests/test_harness.always_bad:
tests/test_harness.always_bad .................................. FAILED (CODE 1)
```

## Your Application

With MOOSE built and testing successfully, you should now be able to build your application. The
only prerequisites before doing so, is to configure your environment to make use of your MPI
wrapper, and to define where MOOSE resides:

```bash
export CC=mpicc CXX=mpicxx FC=mpif90 F90=mpif90 F77=mpif77
export MOOSE_DIR=$HOME/offline/moose
```

You will need to export the above variables each time you open a new terminal window on your HPC
cluster, or personal workstation before you can perform any application development.

For this reason you may wish to add the above export commands to your shell startup profile. This is
different for each shell type (and there are a lot). You'll want to read up on how to do this for
Macintosh (usually ZSH: `~/.zshrc`), or Linux (usually BASH: `~/.bashrc`) respectfully.

You can determine the shell your environment operates within by running the following command:

```bash
echo $0
```

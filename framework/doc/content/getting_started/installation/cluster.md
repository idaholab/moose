# High Performance Computing

The following document will aid an administrator on setting up an environment for which end-users will need when building and running MOOSE based applications.
If you do not have administrative rights, you will not be able to complete these instructions. Please forward these instructions to your HPC administrator.

## Prerequisites

- Some sort of environmental module management software. Such as [Environment Modules](http://modules.sourceforge.net/).
- A working MPI wrapper (MPICH/OpenMPI/MVAPICH) which wraps to a C++11 compliant compiler.
- Knowledge on how to set up the environment to use the desired compiler (either module loading, or exporting PATHs etc)

!alert warning
Please use a single solitary terminal session throughout and to the completion of these instructions.

## Environment Setup

Begin by creating an area for which to build:

```bash
export CLUSTER_TEMP=`mktemp -d /tmp/cluster_temp.XXXXXX`
cd $CLUSTER_TEMP
```

## Set your umask

Some systems have a secure umask set. We need to adjust our umask so that when you write a file, it is readable by everyone:

```bash
umask 0022
```

## Choose a base path

Export a base path variable which will be the home location for the compiler stack. All files related to libraries necessary to build MOOSE, will be stored in this location (+choose carefully, as this location should be accessible from all nodes on your cluster+):

```bash
export PACKAGES_DIR=/opt/moose-compilers
```

## Create and chown $PACKGES_DIR

History teaches us, that implicitly trusting scripts we download off the internet with root access, is a very bad idea. So let us create and chown the $PACKAGES_DIR directory before we install anything. That way, the things we do install can be done so with out invoking `sudo`:

```bash
sudo mkdir $PACKAGES_DIR
sudo chown -R <your user id> $PACKAGES_DIR
```

!alert! warning
Verify that your umask settings are indeed set to 0022 before continuing:

```bash
#> umask
0022
```
!alert-end!

## Setup Modules

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

setenv          PETSC_DIR        $base_path/petsc/petsc-__PETSC_DEFAULT__/gcc-opt

# Optional if miniconda is installed
prepend-path    PATH             $base_path/miniconda/bin
!package-end!

!alert! note
Replace `INSERT PACKAGES_DIR HERE` with whatever you had set for $PACKAGES_DIR (+do not+ literally enter: $PACKAGES_DIR. As an example, if you left packages_dir as: /opt/moose-compilers, then that is what you would enter)

Replace `GCC and MPI` paths with any additional information needed to make GCC/MPI work.
!alert-end!

To make the module available in your terminal session, export the following:

```bash
export MODULEPATH=$MODULEPATH:$PACKAGES_DIR/modulefiles
```

The above command should be added to the list of other global profiles (perhaps in /etc/profiles.d). That way, the above is performed as the user logs into the machine.

With the modulefile in place and the MODULEPATH variable set, see if our module is available for loading:

```bash
module avail
```

Verify that somewhere among all the other modules installed, you see: 'moose-dev-gcc' among them.


## Miniconda

If your users will want to use Peacock (a MOOSE GUI frontend), the easiest way to enable this feature, is to install miniconda, along with several miniconda/pip packages.

```bash
cd $CLUSTER_TEMP
curl -L -O https://repo.continuum.io/miniconda/Miniconda2-latest-Linux-x86_64.sh
sh Miniconda2-latest-Linux-x86_64.sh -b -p $PACKAGES_DIR/miniconda

PATH=$PACKAGES_DIR/miniconda/bin:$PATH conda config --set ssl_verify false
PATH=$PACKAGES_DIR/miniconda/bin:$PATH conda install -c idaholab python=2.7 coverage \
  reportlab \
  mako \
  numpy \
  scipy \
  scikit-learn \
  h5py \
  hdf5 \
  scikit-image \
  requests \
  vtk=7.1.0 \
  pyyaml \
  matplotlib \
  pip \
  lxml \
  pyflakes \
  pandas \
  conda-build \
  mock \
  yaml \
  pyqt \
  swig --yes
```

!alert note
Peacock (as well as the TestHarness sytem in MOOSE), does not work with Python3. Please chose Miniconda2 for Python 2.7 instead.


Next, we need to use `pip` to install additional libraries not supplied by conda:

```bash
PATH=$PACKAGES_DIR/miniconda/bin:$PATH pip install --no-cache-dir pybtex livereload==2.5.1 daemonlite pylint==1.6.5 lxml pylatexenc anytree
```

## Building PETSc

If you opted to build a moose-dev-gcc module, and if you have modules available on your cluster, we will load it as a user would. This will ensure that everything we did above is going to work for our end-users.

```bash
module purge # unload a possible conflicting environment set up by a different module
module load moose-dev-gcc
echo $PETSC_DIR
```

!alert note
Verify that the above `echo $PETSC_DIR` command is returning /the-packages_dir-path/petsc/petsc-!!package petsc_default!! you originally set up during the 'Choose a base path' step. If not, something went wrong with creating the moose-dev-gcc module in the previous step.

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

+During the configure/build process, you will be prompted to enter proper make commands.+ This can be different from system to system, so I leave that task to the reader.

## Clean Up and Chown

Clean all the temporary stuff and change the ownership to root, so no further writes are possible:

```bash
rm -rf $CLUSTER_TEMP
sudo chown root $PACKAGE_DIR
```

This concludes setting up the environment for MOOSE-based development. However you decide to instruct your users on enabling the above environment, each user will need to perform the instructions provided by the following link: [Obtaining and Building MOOSE](getting_started/installation/install_moose.md).

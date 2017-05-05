# HPC Cluster Setup
The following document aims at setting up a baseline multi-user environment for building MOOSE based applications in a job scheduling capable environment.

---
## Prerequisites
Both of these pre-reqs are in the hands of the admins of the cluster.

* Modules. If not already installed. ['Modules Environment'](http://modules.sourceforge.net/) (Or some kind of environment management software)
* Whatever compiler you choose to use on your cluster (GCC/Clang/Intel, MPICH/OpenMPI/MVAPICH), the minimum requirement, is that it must be C++11 compatible. If you are unsure, please consult with your system admins for your cluster on which compiler to use (and how to use it).

---
## Environment Setup
* Begin by creating an area for which to build.

```bash
export CLUSTER_TEMP=`mktemp -d /tmp/cluster_temp.XXXXXX`
cd $CLUSTER_TEMP
```

!!! note
    The terminal you used to run that command, should be the terminal you use from now on while following the instructions to completion


#### Set your umask
* Some systems have a secure umask set. We need to adjust our umask so that everything we are about to perform, is readable/executable by _<b>everyone</b>_ on your cluster:

```bash
umask 0022
```


#### Choose a base path
* Export a base path variable which will be the home location for the compiler stack. All files related to MOOSE will be stored in this location (so choose carefully now):

```bash
export PACKAGES_DIR=/opt/moose-compilers
```

!!! Important
    You can change this path to whatever you want. The only exception, is this path must reside in a location where all your compute nodes have access (nfs|panfs share)

#### Setup Modules
Even if you're not using Modules, the following step should give you an idea of what is needed in the users environment for MOOSE developement.

* Create a MOOSE module

```bash
sudo mkdir -p $PACKAGES_DIR/modulefiles
sudo vi $PACKAGES_DIR/modulefiles/moose-dev-gcc
```

* Add the following content to that file:

```bash
#%Module1.0#####################################################################
##
## MOOSE module
##
set base_path   INSERT PACKAGES_DIR HERE

<some GCC MPI compiler>

setenv CC       mpicc
setenv CXX      mpicxx
setenv F90      mpif90
setenv F77      mpif77
setenv FC       mpif90

setenv          PETSC_DIR       $base_path/petsc/petsc-!include docs/content/getting_started/petsc_default_version.md/gcc-opt
```

!!! note
    You must replace 'INSERT PACKAGES_DIR HERE' with whatever you had set for $PACKAGES_DIR. The bit about 'some GCC MPI compiler', is referring to you having to add any additional environment changes in order make your cluster's MPI implementation work. Normally, this would be loading an additional module (example: module load mvapich2-gcc/1.7)


* To make the module available in your terminal session throughout the rest of the instructions, export the following:
```bash
export MODULEPATH=$MODULEPATH:$PACKAGES_DIR/modulefiles
```

* The above will have to be performed using a more permanent method so that it may be usable by everyone at anytime (including while on a node). Something on the oder of
    1. Copy the moose-dev-gcc module file to where-ever the rest of the system's modules are located
    2. Add the above export command to the system-wide bash profile (using absolute paths instead of $PACKAGES_DIR variable)
    3. Inform the user how to add the above export command to their personal profile (using absolute paths instead of $PACKAGES_DIR variable)

* On our systems, we prefer option 3. Option 3 will allow the moose-dev-gcc module to be easily found, while not being intrusive to other non-MOOSE users (because only your MOOSE user is exporting that MODULEPATH command) Example:
```text
me@some_machine#>  module available

--------------------------------------- /usr/share/modules ---------------------------------------
3.2.10

----------------------------- /usr/share/Modules/3.2.10/modulefiles ------------------------------
dot         module-git  module-info modules     null        use.own

-------------------------------- /apps/local/modules/modulefiles ---------------------------------
intel/12.1.2               python/2.7                 starccm+/7.05.026
intel/12.1.3               python/2.7-open            starccm+/7.05.067
intel-mkl/10.3.8           python/3.2                 starccm+/7.06.012(default)
intel-mkl/10.3.9           python/as-2.7.2            starccm+/8.02.008
mvapich2-gcc/1.7           python/as-3.2              totalview/8.11.0(default)
mvapich2-intel/1.7         starccm+/6.06.011          totalview/8.6.2
pbs                        starccm+/7.02.008          use.projects
pgi/12.4                   starccm+/7.04.006          vtk

-------------------------------- /apps/projects/moose/modulefiles --------------------------------
moose-dev-gcc      moose-dev-gcc-parmesh     moose-dev-clang    moose-tools
me@some_machine#>
```
* The point being, some HPC clusters can have hundreds of available modules. While using this method, you can group and display certain modules pertinent to your MOOSE users at the end of stdout.

---
## Building PETSc
* Now that we have our environment ready, we will load it up as a user would. This will ensure that everything we did above is going to work for our end-users.

```bash
module load moose-dev-gcc
echo $PETSC_DIR
```

!!! note
    Verify that the environment variable 'PETSC_DIR' is available and returning $PACKAGES_DIR/petsc/petsc-!include docs/content/getting_started/petsc_default_version.md. If not, something went wrong with creating the moose-dev-gcc module above.

* Download and extract PETSc:
```bash
curl -L -O http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-!include docs/content/getting_started/petsc_default_version.md.tar.gz
tar -xf petsc-!include docs/content/getting_started/petsc_default_version.md.tar.gz
cd petsc-!include docs/content/getting_started/petsc_default_version.md
```
* Configure PETSc using the following options (see Info admonition below):

!include docs/content/getting_started/petsc_default.md

During the configure/build process, you will be prompted to enter proper make commands. This can be different from system to system, so I leave that task to the reader.

---
## Clean Up
* Clean all the temporary stuff:

```bash
rm -rf $CLUSTER_TEMP
```

!include docs/content/getting_started/installation/clone_moose.md
!include docs/content/getting_started/installation/build_libmesh.md

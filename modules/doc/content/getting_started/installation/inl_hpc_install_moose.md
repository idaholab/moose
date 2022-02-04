# INL HPC Cluster

This page aims at simplifying the installation process on INL computing clusters.
The [general cluster instructions](hpc_install_moose.md) may also be used, but these
instructions will save some steps. There are multiple options for installing / using
MOOSE and MOOSE applications.

!alert note
Falcon is planned to be decommissioned this January 2022.

!alert warning
Do not mix installation workflows except where specified below.

!alert warning
When installing with Option 2 and 3, the installed MOOSE or application will only
function on **one** of the INL clusters. The compilers/MPI distributions are not the same
on all the platforms.

## Option 1: Loading an application directly (easy, users only)

You may load pre-built binaries for MOOSE
and the applications you have access to by running: `module load use.moose moose-apps <app_name>`.
Pre-built executables are then added in the `PATH` environment variable, and can be called
directly in any folder with: `app_name-opt -i <input_file>`.

## Option 2: Modules + source-based installation (medium, for developers)

!alert warning
If any of the modules (mostly the specific version) you used when installing is no longer offered, you will need to
install again. We take every achievable step to avoid this, but it is at times unavoidable on this shared infrastructure.
[Option 3](option3) is slightly more resilient to this as it does not use a PETSc module.

### Step 1: load some compilers

In order to compile MOOSE and its applications, we first need to load compilers & compiling tools.
The following modules are recommended for each of INL's clusters:

- Falcon: `module load use.moose MVAPICH2 CMake`

- Lemhi: `module load use.moose moose-dev`

- Sawtooth: `module load use.moose moose-dev`


!alert note
If you use HPC modules, you will need to load them every time you log in! One solution is to
modify/create the `.bash_profile` file and add these `module load` commands there. However, be careful
that this is not always harmless to other HPC programs, and can interfere with other modules
you would be using on INL HPC.

### Step 2: module-load or download MOOSE dependencies id=step1

PETSc may be obtained directly from HPC modules pre-installed on each machine. Because we want to be
able to add new PETSc modules without breaking anyone's installation of MOOSE, we purposely do not
set a default. You will have first locate a PETSc module of the desired version:

```
  module spider PETSc
```

!alert warning
Make sure to pick a PETSc module built with the same compilers we loaded earlier! This is usually mvapich2. Type
`which mpicc` to check which compilers are loaded. If loading the PETSc module attempts to swap the MPI module,
then with absolute certainty it was not compiled with the compilers we loaded earlier.

Then once you have located the version you wish to use, you will need to explicitly specify this version
in the `module load` command. We will try to never delete old modules. However, when compilers are re-installed
on the cluster for example, we have no choice but to delete broken modules.

- Falcon: `module load use.moose PETSc`

- Lemhi: `module load use.moose PETSc/<version>/<hash>`

- Sawtooth: `module load use.moose PETSc/<version>/<hash>`


!alert note
For some MOOSE applications (Griffin & others), the latest version of `PETSc` is required, in which case
you will need to run the `update_and_rebuild_petsc.sh` script, see [Option 3: Step 2](update_and_rebuild).

! alert note
If you need PETSc compiled with a certain library enabled, with different compilers, or you need a
version of PETSc that is not listed in the modules, please see [Option 3: Step 2](update_and_rebuild).
We cannot cater to special requests for modules.

- libMesh


LibMesh does not have pre-built modules as it is very frequently updated. In order to install, or update,
libMesh, you will need to run the `update_and_rebuild_libmesh.sh` script.

```
  cd scripts
  MOOSE_JOBS=6 ./update_and_rebuild_libmesh.sh
```

### Step 3: download and install MOOSE and the application you need id=step3

MOOSE may be classically obtained from its [GitHub repository](https://github.com/idaholab/moose).
Many open-source MOOSE applications may also be obtained from GitHub, while others will require
officially requesting access through the [NCRC](ncrc/ncrc_ondemand.md). Please then follow the
associated instructions for installation.

## Option 3: Installation solely from source (medium, slower, for developers) id=option3

### Step 1:  See [Option 2: Step 1](step1)

### Step 2: download and install MOOSE dependencies id=update_and_rebuild

MOOSE dependencies, mainly PETSc and libMesh, may be obtained using the `update_and_rebuild_...`
scripts, as is detailed in these [instructions](hpc_install_moose.md). These scripts currently
should be run on the head node, as the compute nodes do not have access to the internet.
Please make sure to `export MOOSE_JOBS=6` to control the number of cores used for installation.

```
  cd scripts
  export MOOSE_JOBS=6
  ./update_and_rebuild_petsc.sh
  ./update_and_rebuild_libmesh.sh
```

### Step 3: see [Option 2: Step 3](step3)

## Option 4: conda installation (not recommended, light users)

!alert warning
The [conda-based installation](conda.md) is not recommended for INL HPC, as it does not use
optimized, architecture-specific compilers. However, if you are not seeking performance,
and will not use more than one node (to limit the attrition of computational resources),
you are free to use them.

!alert note
If you want to use MPI with a conda installation, you will likely need to use the `mpiexec / mpirun`
in the conda package, not another one from a module.

!alert note
Do not use the `miniconda/anaconda` HPC module. Download and install your own conda. If you get permission
errors when trying to install conda packages, it is likely the HPC conda module is being used.
`module unload miniconda` should be ran to remove it.

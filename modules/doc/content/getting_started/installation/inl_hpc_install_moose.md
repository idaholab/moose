# INL HPC Cluster

This page aims at simplifying the installation process on INL computing clusters.
The [general cluster instructions](hpc_install_moose.md) may also be used, but these
instructions will save some steps. There are multiple options for installing / using
MOOSE and MOOSE applications.

!alert note
Falcon is planned to be decommissioned March 31 2022.

!alert warning
Do not mix installation workflows except where specified below.

!alert warning
When installing with Option 2, the installed MOOSE or application will only
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

### Step 2: download MOOSE dependencies id=step1

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

### Step 3: download and install MOOSE and the application you need id=step3

MOOSE may be classically obtained from its [GitHub repository](https://github.com/idaholab/moose).
Many open-source MOOSE applications may also be obtained from GitHub, while others will require
officially requesting access through the [NCRC](ncrc/ncrc_ondemand.md). Please then follow the
associated instructions for installation.

## Option 3: conda installation (not recommended, light users)

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

# Process for building moose-mpich and then MOOSE on ARM64 (M1, Monterey)

!alert! construction title=Work In Progress
This document is intended to be a living one, as M1 support is added and the current
"best practice" of installing it improved. As of Nov 3 2021, full package support
(i.e., `moose-mpich` + `moose-petsc` + `moose-libmesh`) is not yet available.
!alert-end!

!alert! note title=Miniforge3
This guide assumes that you have miniforge3 installed. Download the newest release
[using this link](https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-arm64.sh).
Then the [MOOSE installation instructions for conda](conda.md) can be used to perform
the installation.
!alert-end!

1. Download MacOSX 11.3 SDK [via GitHub](https://github.com/phracker/MacOSX-SDKs/releases/download/11.3/MacOSX11.3.sdk.tar.xz).

2. Extract the SDK and place it in `/opt/`

3. Set up conda build environment:

   ```
   conda deactivate
   conda install mamba
   conda create -n build -y
   conda activate build
   mamba install boa
   ```

   Deactivate and re-activate to ensure environment is in the proper state:

   ```
   conda deactivate
   conda activate build
   ```

4. Build moose-mpich and moose-tools locally (make sure build environment is activated) and init submodules

   ```
   cd ~/projects/moose/conda
   conda mambabuild mpich
   conda mambabuild tools
   cd ..
   git submodule update --init --recursive petsc libmesh
   ```

5. Create development environment and build PETSc:

   ```
   conda deactivate
   conda create -n testing -y
   conda activate testing
   mamba install -c ~/miniforge3/envs/build/conda-bld moose-mpich moose-tools cmake
   ```

   !alert! tip
   IMPORTANT: Close and re-open the Terminal so that the environment variables
   can be properly set upon activation
   !alert-end!

   ```
   conda activate testing
   cd ~/projects/moose
   scripts/updata_and_rebuild_petsc.sh --enable-shared-libraries=0 --download-mumps=0
   ```

   The modified configure options are necessary for the current build to succeed.
   FBLASLAPACK fails if shared libraries are enabled, and the `-lmpiseq` library
   cannot be currently found in the current `moose-mpich` build on Monterey.

6. When PETSc completes, libmesh and its dependencies need to be bootstrapped for ARM64 on MacOS:

   ```
   cd libmesh
   git submodule update --recursive
   ./bootstrap
   cd contrib/metaphysicl
   ./bootstrap
   cd ../timpi
   ./bootstrap
   cd ../netcdf/netcdf*
   autoreconf
   cd ../../../.. # to get back to moose
   ```

7. Then run the libmesh script as normal:

   ```
   scripts/update_and_rebuild_libmesh.sh
   ```

8. Build and test MOOSE:

   ```
   cd test
   make -j8
   ./run_tests -j8
   ```

!alert! warning title=Failing tests
This method of building MOOSE *will* result in many failing tests. Currently (as
of Nov 3 2021), many of these tests fail with the following error:

```
dyld[75720]: symbol not found in flat namespace '_petsc_allreduce_ct'
```
!alert-end!

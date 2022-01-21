# Process for building moose-mpich and then MOOSE on ARM64 (M1, Monterey)

!alert! construction title=Work In Progress
This document is intended to be a living one, as M1 support is added and the current
"best practice" of installing it improved. As of 21 Jan 2022, full, public package
support (i.e., `moose-mpich` + `moose-petsc` + `moose-libmesh`) is not yet available.
!alert-end!

!alert! note title=Mambaforge3
This guide assumes that you have mambaforge3 installed. The [MOOSE installation instructions for conda](conda.md)
can be used to perform the installation. Note for those instructions that `x86_64`
in the name of the Mambaforge install script should be replaced with `arm64`.
!alert-end!

1. Download MacOSX 11.3 SDK [via GitHub](https://github.com/phracker/MacOSX-SDKs/releases/download/11.3/MacOSX11.3.sdk.tar.xz).

2. Extract the SDK and place it in `/opt/`

3. Set up conda build environment:

   ```
   conda deactivate
   conda create -n build -y
   conda activate build
   mamba install boa
   ```

   Deactivate and re-activate to ensure environment is in the proper state:

   ```
   conda deactivate
   conda activate build
   ```

4. Initialize MOOSE submodules (as well as all libMesh contrib submodules):

   ```
   git submodule update --init petsc libmesh
   cd ~/projects/moose/libmesh
   git submodule update --init --recursive
   ```

5. Build MOOSE packages locally and in order of dependency (this step will take some time):

   ```
   cd ~/projects/moose/conda
   conda mambabuild mpich
   conda mambabuild libmesh-vtk
   conda mambabuild petsc
   conda mambabuild libmesh
   conda mambabuild tools
   ```

6. Create development environment (can name it something other than "testing" if you want):

   ```
   conda deactivate
   conda create -n testing -y
   conda activate testing
   mamba install -c ~/mambaforge3/envs/build/conda-bld moose-libmesh moose-tools
   ```

   !alert! tip
   IMPORTANT: Close and re-open the Terminal so that the environment variables
   can be properly set upon activation
   !alert-end!

7. Build and test MOOSE:

   ```
   conda activate testing
   cd ~/projects/moose/test
   make -j8
   ./run_tests -j8
   ```

!alert! warning title=Failing tests
This method of building MOOSE should pass all framework and unit tests in the `opt`
build mode. Currently (as of 21 Jan 2022), only a handful of geochemistry tests
are failing. "Heavy" tests still need to be tested more thoroughly.  
!alert-end!

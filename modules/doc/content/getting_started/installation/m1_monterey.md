# Process for building moose-mpich and then MOOSE on ARM64 (M1, Monterey)

NOTE: This assumes that you have miniforge3 installed. (https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-arm64.sh). The MOOSE installation instructions for conda can be used to do this.

1. Download MacOSX 11.3 SDK (https://github.com/phracker/MacOSX-SDKs/releases/download/11.3/MacOSX11.3.sdk.tar.xz)

2. Extract the SDK and place it in /opt/

3. Set up conda build environment:

```
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

IMPORTANT: Close and re-open the Terminal


```
conda activate testing
cd ~/projects/moose
scripts/updata_and_rebuild_petsc.sh --enable-shared-libraries=0 --download-mumps=0
```

The modified configure options are necessary for the current build to succeed.
FBLASLAPACK fails if shared libraries are enabled, and the `-lmpiseq` library cannot
be currently found in the current `moose-mpich` build on Monterey.

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

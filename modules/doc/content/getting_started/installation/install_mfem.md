# Installing MFEM-MOOSE

To enable `MFEM-MOOSE` capabilities, it is necessary to install all of its dependencies, including `MFEM` itself and `Conduit`. You may do so by performing the following steps:

1. After cloning the `MOOSE` repository, navigate to the root directory and run the following commands:

```bash
export METHOD=opt
export MOOSE_JOBS=10
```

First, we need to build `PETSc`. To do that, we can run the `PETSc` installer script:

```bash
./scripts/update_and_rebuild_petsc.sh [options]
```

You may substitute `[options]` with any `PETSc` configure flags, which are discussed in the [`PETSc` install page](https://petsc.org/release/install/install). If you intend to build `MFEM-MOOSE` with GPU offloading capabilities, make sure to include `--with-cuda` or `--with-hip`. If desired, you may also specify the GPU architecture with `--with-cuda-arch=[arch]` or `--with-hip-arch=[arch]`. To maximize performance and avoid errors, you should also make sure that your underlying MPI library is GPU-aware.

!alert note
The `PETSc` GPU architecture specification takes in only the architecture number for `CUDA` builds, for instance for the `sm_80` architecture you would add the flag `--with-cuda-arch=80`. For `HIP` builds you should use the entire label, for instance `--with-hip-arch=gfx908`.


Next, we build `libMesh`, `Conduit`, and `Wasp` by running their respective scripts:

```bash
./scripts/update_and_rebuild_libmesh.sh
./scripts/update_and_rebuild_conduit.sh
./scripts/update_and_rebuild_wasp.sh
```

We must then build `MFEM` itself by running its script:

```bash
./scripts/update_and_rebuild_mfem.sh [options]
```

Here, in `[options]`, you may wish to include `MFEM`'s cmake options, which are detailed [in this page](https://github.com/mfem/mfem/blob/master/INSTALL). Notably, like with the `PETSc` install, if you are building `MOOSE-MFEM` with GPU offloading capabilities, here your flags should include `-DMFEM_USE_CUDA=YES` or `-DMFEM_USE_HIP=YES`. For the GPU architecture specification, use `-DCUDA_ARCH=[arch]` or `-DHIP_ARCH=[arch]`.

!alert note
The `MFEM` GPU architecture specification takes in the entire label, for instance `-DCUDA_ARCH=sm_80`, or for a `HIP` build, you may use `-DHIP_ARCH=gfx908`.

!alert note
When building `MFEM` with GPU support, there is a known bug whereby the configure step may return an error claiming not to have been able to find a valid version of `SuperLU_DIST`, despite the user (or in this case the install script) providing one. If you encounter this error, you can bypass it by passing the `MFEM` flag `-DSuperLUDist_VERSION_OK=YES` to the build script.

!alert tip
Alternatively, if you already have a working `MFEM` build in a separate directory, you may set the variable `MFEM_DIR` to its install path instead of running the `MFEM` install script.


2. Configure the `MOOSE` build by running

```bash
./configure --with-mfem
```

You may also wish to include other `MOOSE` configuration flags as necessary.

3. Finally, build the framework, module, and tests by running

```bash
cd framework
make -j $MOOSE_JOBS
cd ../modules
make -j $MOOSE_JOBS
cd ../test
make -j $MOOSE_JOBS
```

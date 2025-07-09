# Kokkos

The Kokkos capability of MOOSE acquires the Kokkos library through PETSc, so it requires PETSc to be configured with Kokkos support. You may achieve it by performing the following steps:

1. After cloning the MOOSE repository, navigate to the root directory and run the following commands (you may replace `10` with however many cpus you wish to use for compilation):

```bash
export METHOD=opt
export MOOSE_JOBS=10
```

Then, build PETSc by running the PETSc installer script:

```bash
./scripts/update_and_rebuild_petsc.sh --with-cuda --with-cuda-arch=[arch] --download-kokkos --download-kokkos-kernels --download-slate [options]
```

The current Kokkos capability of MOOSE only supports CUDA for NVIDIA GPUs to be the backend for Kokkos, and the support for HIP and SYCL for AMD and Intel GPUs will be added in the future. Kokkos can only be built with a single GPU architecture at a time, so your GPU architecture should be specified in the place of `[arch]`. To maximize performance and avoid errors, you should also make sure that your underlying MPI library is GPU-aware. You may substitute `[options]` with any other desired PETSc configure flags, which are discussed in the [PETSc install page](https://petsc.org/release/install/install).

!alert note
The PETSc GPU architecture specification takes in only the architecture number for CUDA builds, for instance for the `sm_80` architecture you would add the flag `--with-cuda-arch=80`.


Next, we build libMesh and Wasp by running their respective scripts:

```bash
./scripts/update_and_rebuild_libmesh.sh
./scripts/update_and_rebuild_wasp.sh
```

2. Configure the MOOSE build by running

```bash
./configure --with-kokkos
```

You may also wish to include other MOOSE configuration flags as necessary.

3. Finally, build the framework, module, and tests as desired by running

```bash
cd framework
make -j $MOOSE_JOBS
cd ../modules
make -j $MOOSE_JOBS
cd ../test
make -j $MOOSE_JOBS
```

Use [this page](syntax/Kokkos/index.md) as your starting point for learning how to use the Kokkos capability in MOOSE.

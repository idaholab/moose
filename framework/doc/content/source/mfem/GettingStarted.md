# Getting started with `MFEM-MOOSE`

## Introduction

`MFEM-MOOSE` is a part of the `MOOSE` framework which utilizes [`MFEM`](https://mfem.org) as its main Finite Element Method backend. It expands upon `MOOSE`'s base capabilities to support functionalities including but not limited to:

- High-order H(Div) and H(Curl) elements
- Various assembly levels (including matrix-free)
- GPU offloading (`CUDA` and `HIP`)
- Low-Order-Refined solvers


## Installing `MFEM-MOOSE`

To enable `MFEM-MOOSE` capabilities, it is necessary to install all of its dependencies, including `MFEM` itself and `Conduit`. You may do so by performing the following steps:

1. After cloning the `MOOSE` repository, navigate to the root directory and run the following commands:

```bash
export METHOD=opt

./scripts/update_and_rebuild_petsc.sh
./scripts/update_and_rebuild_libmesh.sh
./scripts/update_and_rebuild_mfem.sh
./scripts/update_and_rebuild_conduit.sh
./scripts/update_and_rebuild_wasp.sh
```

It may be necessary to include your desired configuration flags (for instance `--with-mpi`) to each script invocation. Alternatively, if you already have working `MFEM` or `Conduit` builds in a separate directory, you may set the variables `MFEM_DIR` and `CONDUIT_DIR` to their respective paths.

2. Configure the `MOOSE` build by running

```bash
./configure --with-mfem
```

Again, you may wish to include other configuration flags.

3. Finally, build the framework, module, and tests by running

```bash
NUM_THREADS=10

cd framework
make -j $NUM_THREADS
cd ../modules
make -j $NUM_THREADS
cd ../test
make -j $NUM_THREADS
```

## Solving a problem with `MFEM-MOOSE`




!if-end!

!else
!include mfem/mfem_warning.md
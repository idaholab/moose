# Libtorch (PyTorch C++ API)

MOOSE can be linked against [libtorch](https://pytorch.org/cppdocs/) [!cite](paszke2019pytorch) to enable hardware acceleration and some high-level machine-learning capabilities. The libtorch source is provided as a git submodule under `framework/contrib/pytorch`, and a setup script is provided to build and install it with the configuration MOOSE expects.

## Prerequisites

The libtorch build depends on [PETSc](https://petsc.org) so that BLAS and LAPACK are consistent between the two libraries. PETSc must therefore be installed before libtorch. The script looks for PETSc at `${PETSC_DIR}` (defined if you are using conda, and otherwise defaults to `<MOOSE_DIR>/petsc/arch-moose`) and aborts if it cannot be found.

A working compiler stack and CMake are also required. The [MOOSE conda environment](installation/conda.md) satisfies both.

## Installation

To build and install libtorch, run

```bash
cd ~/projects/moose
./scripts/update_and_rebuild_libtorch.sh
```

!alert tip
The setup script uses sensible defaults that work out-of-the-box. The script is also extensively customizable. Use the `--help` argument to print out a detailed help message.

The most commonly used options and environment variables are:

- `--fast` — skip the update, clean, and configure steps (re-build only).
- `--skip-submodule-update` — skip the submodule update step.
- `LIBTORCH_DIR` — installation directory. Defaults to `<MOOSE_DIR>/framework/contrib/pytorch/installed`.
- `LIBTORCH_SRC_DIR` — use a custom libtorch source tree instead of the bundled submodule. Setting this implies `--skip-submodule-update`.
- `LIBTORCH_JOBS` — number of parallel build jobs. Defaults to `MOOSE_JOBS`, or 1 if unset.
- `PETSC_DIR` — path to the PETSc installation.

!alert! note title=GPU support
CUDA is enabled automatically when a CUDA toolkit is detected on the system. For Intel GPU (XPU) support, export `USE_XPU=1` before invoking the script. General CMake environment variables are also respected, so other backends can be toggled by passing additional `-D...` arguments through to the configure step.
!alert-end!

Building libtorch is resource-intensive: expect a multi-gigabyte build directory and a long compile time on the first build. Subsequent rebuilds can be sped up with `--fast`.

## Configuring MOOSE with libtorch

Once libtorch has been installed, configure MOOSE with

```bash
./configure --with-libtorch
```

If libtorch was installed to a non-default location, pass that path explicitly:

```bash
./configure --with-libtorch=/path/to/libtorch
```

## HPC systems

On INL HPC systems, libtorch is already provided through the `moose-dev` module/container and does not need to be built manually. See the [INL HPC installation instructions](inl_hpc_install_moose.md) for more information. On other HPC systems, follow the same procedure as for a workstation, taking care to load a compatible compiler stack and CMake before invoking the script.

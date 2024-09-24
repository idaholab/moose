# Libtorch (Pytorch C++ API)

The way one enables LibTorch [!cite](paszke2019pytorch) capabilities in MOOSE depends on
the operating system (Linux or Mac) and if we use HPC or just a local workstation.

!alert! note title=Compatibility

Before trying to install MOOSE with libtorch make sure to check out the
[compatibility matrix](https://github.com/pytorch/pytorch/blob/main/RELEASE.md#release-compatibility-matrix)
which will tell you if the required packages are compatible with the ones required by MOOSE.
Furthermore, we do not support `libtorch` versions below v1.4.

!alert-end!

## Mac workstations

For Mac workstations, the user has two distinct paths for enabling libtorch within MOOSE.
Both paths rely on conda for installing the moose environment, therefore we recommend the
users follow [Conda MOOSE Environment](installation/conda.md) instructions before starting
this process.

!alert! note title=GPU Support on Macs

For ARM Mac workstations, both installation procedures will give access
to the Metal Performance Shader (MPS) capabilities (GPU acceleration).

!alert-end!

### Using packages distributed through conda

1. Create and activate a new MOOSE environment with pytorch included:

   ```bash
   conda create -n moose-torch moose-dev pytorch=2.1 -c pytorch
   conda activate moose-torch
   ```

   This will provide the headers and libraries needed to use libtorch.

2. Configure MOOSE to use libtorch. The user needs to link to the conda-based libtorch libraries
   using the following command within the root directory of `moose`:

   ```bash
   ./configure --with-libtorch=${CONDA_PREFIX}/lib/python3.11/site-packages/torch
   ```

   !alert! note title=How to get the pytorch directory

   The python version can be different depending on the distribution, so make sure you double-check if the directory you point to actually exists!
   An easy way to find if the library exists within the conda package is running the following command in the terminal:

   ```bash
   find ${CONDA_PREFIX} -type d -name torch
   ```

   Alternatively, you can use python to get the same directory:

   ```bash
   python -c "import torch; print(torch.__path__[0])"
   ```

   !alert-end!

3. Once moose has been configured to work with libtorch, we need to recompile MOOSE.
   For testing purposes, we can do the following:

   ```bash
   cd test
   make -j 8
   ./run_test --re=libtorch
   ```

   If you see tests passing with green colors, the installation was successful.

### Using pre-compiled packages from the official website

1. Create and activate a MOOSE environment without pytorch:

   ```bash
   conda create -n moose moose-dev
   conda activate moose
   ```

2. Pull the precompiled libraries using the setup script provided in MOOSE. From the root
   directory of `moose` this is done by executing the following command:

   ```
   ./scripts/setup_libtorch.sh --version=2.1
   ```

   !alert! note title=Version limitations for ARM Workstations

   The official distribution system does not include packages for ARM architectures up until version 2.2. If the user want to use this path on ARM machines, the version parameter to the script needs
   to be altered.

   !alert-end!

3. Configure MOOSE to use libtorch within the root directory of `moose`:

   ```bash
   ./configure --with-libtorch
   ```

   In this case, the path is not added explicitly considering that the libtorch packages will be
   pulled to the default location.

4. Once moose has been configured to work with libtorch, we need to recompile MOOSE.
   For testing purposes, we can do the following:

   ```bash
   cd test
   make -j 8
   ./run_test --re=libtorch
   ```

   If you see tests passing with green colors, the installation was successful.

## Linux Workstations

For linux, due to the official conda distribution of pytorch is using pre-CXX11 ABI, while the
conda compiler stack of MOOSE relies on this ABI, we don't support conda-based installations yet.

!alert! note title=Main limitation on Linux machines

It is important to emphasize that
linking MOOSE with libtorch on +Linux machines+ is not supported if the compiler stack has been built
using a `libc` version below 2.29 (for `libtorch v 2.1+`) or 2.27 (for `libtorch v 1.8-2.1`)
or 2.23 (for `libtorch v1.4-1.8`). To check your currently used libc version on +Linux machine+, use the following command:

```bash
ldd --version
```

This can be a problem when using the moose conda environment with new versions of libtorch.

!alert-end!

### Using pre-compiled packages from the official website

1. Create and activate a MOOSE environment without pytorch:

   ```bash
   conda create -n moose moose-dev
   conda activate moose
   ```

2. Pull the precompiled libraries using the setup script provided in MOOSE. From the root
   directory of `moose` this is done by executing the following command:

   ```
   ./scripts/setup_libtorch.sh --version=2.1
   ```

3. Configure MOOSE to use libtorch within the root directory of `moose`:

   ```bash
   ./configure --with-libtorch
   ```

4. Once moose has been configured to work with libtorch, we need to recompile MOOSE.
   For testing purposes, we can do the following:

   ```bash
   cd test
   make -j 8
   ./run_test --re=libtorch
   ```

   If you see tests passing with green colors, the installation was successful.

### Manually installing packages

When encountering GLIBC-related compatibility issues on Linux machines the user has two options:

1. [Rebuilding Petsc and libMesh manually](gcc_install_moose.md) using compatible compilers (most newer systems
   like Ubuntu come with compatible compilers).

2. Building libtorch from source. The user can find instructions on how to install libtorch
   from source on the [official website](https://github.com/pytorch/pytorch/blob/master/docs/libtorch.rst).


!alert! note title=GPU Support on Linux Workstations

For linux workstations, we recommend [manually building the dependencies of MOOSE](gcc_install_moose.md)
using suitable system compilers. At the time these instruction are written, only `cuda`-based
acceleration is tested.
The following packages need to be also installed to enable this feature:

- [A sutiable Nvidia driver](https://www.nvidia.com/en-us/drivers/)
- [Cuda toolkit](https://developer.nvidia.com/cuda-toolkit) - only strictly required if
   building libtorch from source.

The supported versions can be determined using the [compatibility matrix](https://github.com/pytorch/pytorch/blob/main/RELEASE.md#release-compatibility-matrix).
Once the dependencies of MOOSE are installed, we can use the setup script to fetch
the correct libtorch package from the official ditribution:

```
./scripts/setup_libtorch.sh --version=2.1 --libtorch-distribution=cuda
```

The configuration and build parts of the process are the same as discussed before.

!alert-end!

## HPC systems

On non-INL HPC systems, one can follow the manual installation process discussed above.
On INL machines, containers are provided with readily compiled dependencies, including libtorch.
For more information on containers, see the [instuctions](inl_hpc_install_moose.md).
In this case, the `moose-dev` module already contains `libtorch`.

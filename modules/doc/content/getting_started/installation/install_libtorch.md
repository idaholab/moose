# Libtorch (Pytorch C++ API)

The way one enables LibTorch [!cite](paszke2019pytorch) capabilities in MOOSE depends on
the operating system (Linux or Mac) and if we use HPC or just a local workstation.

!alert! note
Before we review the main approaches, it is important to emphasize that
linking MOOSE with libtorch on +Linux machines+ is not supported if the compiler stack has been built
using a `libc` version below 2.27 (for `libtorch v 1.8+`)
or 2.23 (for `libtorch v1.4-1.8`). Furthermore, we do not support `libtorch` versions below
v1.4. To check your currently used libc version on +Linux machine+, use the following command:

```bash
ldd --version
```

!alert-end!

## Setting up the environment

For both Mac and Linux workstations, the user needs to create a conda environment using the
instructions [here](installation/conda.md).

## Installing Libtorch

The user can choose from three alternatives when it comes to installing `libtorch`:

- +Install using conda:+

  For ARM and Intel Mac workstations the user can install libtorch using conda together
  with the MOOSE packages:

  ```bash
  conda create -n moose-torch moose-dev pytorch
  ```

  This will provide the headers and libraries needed to use libtorch.

  !alert! note

  The same process works for Linux workstations if the user requires CPU support only.
  For notes on the GPU support, navigate to the bottom of this site.

  !alert-end!

- +Install using the script provided in MOOSE:+

  For this, navigate to the MOOSE root directory and execute the following script:

  ```bash
  ./scripts/setup_libtorch.sh
  ```

  which downloads `libtorch` from the official site and sets it up in the `framework\contrib`
  directory.

  !alert! note

  The script checks for operating system and `libc` version (on Linux workstations)
  and throws errors if the system is not suitable for the coupling. If there is a mismatch
  between the detected `GLIBC` version and the one used to compile libtorch, the user needs
  to install `libMesh` and `PETSc` manually with a compatible compiler.
  !alert-end!

  !alert! note

  The desired version of libtorch can be set by the following argument:

  ```bash
  ./scripts/setup_libtorch.sh --version=1.8
  ```

  Note that we do not support `libtorch` below a version of 1.4. The default
  version downloaded by the script is 1.10.

  !alert-end!

- +Install from source:+
  The user can find instructions on how to install libtorch from source on the
  [official website](https://github.com/pytorch/pytorch/blob/master/docs/libtorch.rst).


## Configure and MOOSE with libtorch

To achieve this, first configure MOOSE with `libtorch` support (along with any other desired configure options)
from within the `moose` folder:

```bash
./configure --with-libtorch
```

!alert! note
If you would like to use a custom libtorch distribution or a manually compiled
version, the destination of the libtorch directory can be supplied to the
configure script by

```bash
./configure --with-libtorch=/path/to/my/custom/libtorch
```

Also note that the library files (.so/.dylib) and the headers in the custom
libtorch installation should follow that of the official distribution handled
by the `setup_libtorch.sh` script.

!alert-end!

For conda-based installations the user can link to the conda-based libtorch libraries
using the approach above (using a typical installation path within conda):


./configure --with-libtorch=/yourcondadirector/mambaforge3/envs/moose-torch/lib/python3.10/site-packages/torch

```bash
./configure --with-libtorch=${CONDA_PREFIX}/lib/python3.10/site-packages/torch
```

!alert! note
The python version can be different depending on the distribution, so make sure you double-check if
the directory you point to actually exists!
!alert-end!

## Build MOOSE with libtorch

The last step is to compile MOOSE with libtorch support:

```bash
make -j 8
```

After configured, compile MOOSE as normal.

!alert! note
Due to namespace conflicts between MOOSE and `libtorch`, the folders of source files
containing libtorch includes need to be excluded from the unit build directories.
The makefile of the [Stochastic Tools Module](stochastic_tools/stochastic_tools.mk)
serves as a good example on how to achieve this for applications.

!alert-end!

## GPU Support

When using ARM Macs the conda-based installation supports both CPU and GPU devices.
For GPU acceleration through Metal, users need to select MPS as a device when writing source code.

!alert! warning

GPU devices on Linux machines are not officially supported yet. If you have questions regarding
this path, feel free to ask them on out Discussion forum!

!alert-end!

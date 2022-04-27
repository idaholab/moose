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

For Mac workstations, the user needs to create a conda environment using the
instructions [here](installation/conda.md). On Linux machines, however,
we cannot use the conda packages due to the mismatch between `libc` versions.
For this reason, given that the system `libc` version allows the linking between
the two libraries, we need to install `PETSc` and `libmesh` manually. For instructions
on the installation of these, see [installation/hpc_install_moose.md].

In situations when the `libc` version allows the linking but the compiler stack
has been compiled with an older `libc` version (HPC machines potentially), we need to build the
compiler from scratch. For instructions in this process, visit [installation/manual_installation_gcc.md]

## Installing Libtorch

The user can choose from two alternatives when it comes to installing `libtorch`:

- +Install using the script provided in MOOSE:+

  For this, navigate to the MOOSE root directory and execute the following script:

  ```bash
  ./scripts/setup_libtorch.sh
  ```

  which downloads `libtorch` from the official site and sets it up in the `framework\contrib`
  directory. The script checks for operating system and `libc` version and throws errors
  if the system is not suitable for the coupling.

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


## Configure and compile MOOSE with libtorch

To achieve this, first configure MOOSE with `libtorch` support (along with any other desired configure options)

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

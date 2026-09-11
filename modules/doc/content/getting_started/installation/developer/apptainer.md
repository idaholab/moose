# Developer Install: Apptainer

!alert note title=Feedback welcome
These instructions were recently updated. If anything is unclear or you have suggestions for improvement, please share your feedback in this [GitHub discussion](https://github.com/idaholab/moose/discussions/32966).

The following instructions describe the use of [Apptainer](https://apptainer.org/) to build and execute MOOSE and MOOSE-based applications within pre-built containers that contain all necessary dependencies. This install method also contains pre-built versions of the optional dependencies libTorch, MFEM, and NEML2.

This is the preferred method of building MOOSE and MOOSE-based applications on Linux.

!alert! note title=Use on HPC
The use of these environments in a HPC environment across nodes (i.e., using MPI to run a job on multiple nodes) is not supported out of the box. You will need your system administrator to setup cross-node MPI by binding in the host MPI into the container.

This is supported on INL HPC in which case you should follow the instructions in [installation/developer/inl_hpc.md] instead.
!alert-end!

To begin, follow these instructions:

1. [#enter_the_environment]: Enter the development environment that contains the necessary dependencies for building and running an application.
1. [#build_and_test]: Build and test an application.

Additional information is provided in the following sections:

- [#non_shell_execution]: Execute an application or command in the development environment without entering a shell within the container.
- [#additional_usage]: Describes additional options that can be provided to Apptainer when using the development environment.
- [#container_listing]: Describes all of the public containerized environments available for execution.

## Enter the Environment id=enter_the_environment

Multiple containerized environments exist for use. By default, we will assume the use of the `moose-dev-openmpi-x86_64` container (see [#container_listing] for a full list of available containerized environments). This container is sufficient for most use cases.

To build and execute MOOSE and MOOSE-based applications, all that is needed is to enter the containerized environment. To enter an interactive shell inside the the containerized development environment, run the following:

!versioner! code
apptainer shell oras://ghcr.io/idaholab/apptainer/moose-dev-openmpi-x86_64:__VERSIONER_VERSION_MOOSE_DEV__
!versioner-end!

If a message like "apptainer: command not found" is emitted, use your system package manager (`yum`, `dnf`, `apt-get`, etc) to install [Apptainer](https://apptainer.org/) or request that is installed by your system administrator. On INL HPC systems, apptainer is made available via a module (i.e., `module load apptainer`).

At this point, you are inserted within an interactive shell inside of the containerized development environment and all of the necessary dependencies for building MOOSE and MOOSE-based applications are available.

!alert note
Applications built within the containerized environment can only be ran within the containerized environment. Thus, if you build an application within the shell and later forget to execute it within the shell, the application will fail to run.

## Build and Test id=build_and_test

After following the instructions to enter the environment in [#enter_the_environment], we will next build and test MOOSE or a MOOSE-based application.

The commands commands that follow +must+ be ran within the `apptainer shell` environment described above in [#enter_the_environment].

Follow the instructions in [#build_and_test_moose] if you are building MOOSE. Otherwise, follow the instructions in [#build_and_test_moose_application].

### Build and Test: MOOSE id=build_and_test_moose

!template load file=installation/developer/includes/build_test_moose.md.template moose_jobs=4

### Build and Test: MOOSE Application id=build_and_test_moose_application

!template load file=installation/developer/includes/build_test_moose_app.md.template moose_jobs=4

## Non-shell Execution id=non_shell_execution

The instructions provided in [#enter_the_environment] allow you to enter a shell environment within the containerized development environment. You can also execute something within this environment without entering the shell (like running a built MOOSE application).

For this, you can utilize the `apptainer exec` command, for example:

!versioner! code
apptainer exec oras://ghcr.io/idaholab/apptainer/moose-dev-openmpi-x86_64:__VERSIONER_VERSION_MOOSE_DEV__ /path/to/app-opt -i input.i
!versioner-end!

This will execute the command:

```bash
/path/to/app-opt -i input.i
```

within the containerized development environment.

## Additional Usage id=additional_usage

The `apptainer shell` and `apptainer exec` commands described above are commonly augmented in the following ways:

### Use a Different Container

The container name `moose-dev-openmpi-x86_64` can be changed another container, for example, `moose-dev-cuda-openmpi-x86_64`, to use the environment that has CUDA available. For example:

!versioner! code
apptainer shell oras://ghcr.io/idaholab/apptainer/moose-dev-cuda-openmpi-x86_64:__VERSIONER_VERSION_MOOSE_DEV__
!versioner-end!

### Use NVIDIA GPUs

GPUs are not available by default in the containerized environment. The command line argument `--nv` will bind in the necessary context from the host systems to enable the use of NVIDIA GPUs in the containerized environment. For example:

!versioner! code
apptainer shell --nv oras://ghcr.io/idaholab/apptainer/moose-dev-cuda-openmpi-x86_64:__VERSIONER_VERSION_MOOSE_DEV__
!versioner-end!

### Bind Mount Directories

By default, Apptainer will make available the your home directory in the environment, and the current directory you are in. Other directories from the host system may not be available.

Let's say that you have a directory `/data` on your system that you want to access within the containerized development environment. By default, it will not be available. To "bind mount" (mount the directory into the containerized environment) directories, the command line option `-B` is used. For example, to bind mount `/data` into the containerized environment, you would run:

!versioner! code
apptainer shell -B /data oras://ghcr.io/idaholab/apptainer/moose-dev-openmpi-x86_64:__VERSIONER_VERSION_MOOSE_DEV__
!versioner-end!

### Using Other Versions

The suffix in the container URI (the `oras://..` component) is the container "tag". This tag represents the version. Here, we use the current version which is [!versioner!version package=moose-dev]. This tag can be replaced with an older tag to use an older version, which can be required if using a version of MOOSE that is not the latest version.

## Container Listing id=container_listing

Multiple containerized environments are available for execution:

- [moose-dev-openmpi-x86_64:[!versioner!version package=moose-dev]](https://github.com/idaholab/moose/pkgs/container/apptainer%2Fmoose-dev-openmpi-x86_64)

  - [RockyLinux](https://rockylinux.org/) [!package!rocky8_apptainer] operating system
  - [OpenMPI](https://www.open-mpi.org/) [!package!openmpi_apptainer], installed at `$MOOSE_OPENMPI_DIR=/opt/openmpi`

- [moose-dev-mpich-x86_64:[!versioner!version package=moose-dev]](https://github.com/idaholab/moose/pkgs/container/apptainer%2Fmoose-dev-mpich-x86_64)

  - [RockyLinux](https://rockylinux.org/) [!package!rocky8_apptainer] operating system
  - [MPICH](https://www.mpich.org/) [!package!mpich_rocky8_apptainer], installed at `$MOOSE_MPICH_DIR=/opt/mpich`

- [moose-dev-rocky9-mpich-x86_64:[!versioner!version package=moose-dev]](https://github.com/idaholab/moose/pkgs/container/apptainer%2Fmoose-dev-rocky9-mpich-x86_64)

  - [RockyLinux](https://rockylinux.org/) [!package!rocky9_apptainer] operating system
  - [MPICH](https://www.mpich.org/) [!package!mpich_rocky9_apptainer], installed at `$MOOSE_MPICH_DIR=/opt/mpich`

- [moose-dev-cuda-openmpi-x86_64:[!versioner!version package=moose-dev]](https://github.com/idaholab/moose/pkgs/container/apptainer%2Fmoose-dev-cuda-openmpi-x86_64)

  - [RockyLinux](https://rockylinux.org/) [!package!rocky8_apptainer] operating system
  - [OpenMPI](https://www.open-mpi.org/) [!package!openmpi_apptainer], installed at `$MOOSE_OPENMPI_DIR=/opt/openmpi`
  - [CUDA](https://developer.nvidia.com/cuda) [!package!cuda_apptainer], installed at `$CUDA_DIR=/usr/local/cuda`
  - CUDA aware builds of OpenMPI, PETSc, libTorch, NEML2, and MFEM

All of the containerized environments above contain the following installed packages:

!table
| Package | Version | Location |
| - | - | - |
| [code-server](https://github.com/coder/code-server) | [!package!code_server_apptainer] | `/usr/bin/code-server` |
| [Conduit](https://llnl-conduit.readthedocs.io) | [!git!submodule-hash length=7 url=https://github.com/llnl/conduit/tree](framework/contrib/conduit) | `$CONDUIT_DIR=/opt/conduit` |
[libMesh](https://libmesh.github.io/) | [!git!submodule-hash length=7 url=https://github.com/libMesh/libmesh/tree](libmesh) | `$LIBMESH_DIR=/opt/libmesh` |
| [libTorch](https://docs.pytorch.org/cppdocs/) | v[!package!libtorch_apptainer] | `$LIBTORCH_DIR=/opt/libtorch` |
| [GCC](https://gcc.gnu.org/) | [!package!gcc_apptainer] | `/opt/rh` |
| [MFEM](https://mfem.org) | [!git!submodule-hash length=7 url=https://github.com/mfem/mfem/tree](framework/contrib/mfem) | `$MFEM_DIR=/opt/mfem` |
| [NEML2](https://applied-material-modeling.github.io/neml2/) | [!git!submodule-hash length=7 url=https://github.com/applied-material-modeling/neml2/tree](framework/contrib/neml2) | `$NEML2_DIR=/opt/neml2` |
| [PETSc](https://petsc.org/) | [!versioner!version prefix=v url=https://gitlab.com/petsc/petsc/-/tree/ package=petsc] | `$PETSC_DIR=/opt/petsc` |
| [Python](https://www.python.org/) | [!package!python_apptainer] | `/opt/miniforge3/envs/moose/bin/python` |
| [VTK](https://vtk.org/) | [!package!vtk] | `$VTK_DIR=/opt/vtk` |
| [WASP](https://code.ornl.gov/neams-workbench/wasp) | [!git!submodule-hash length=7 url=https://code.ornl.gov/neams-workbench/wasp/-/tree](framework/contrib/wasp) | `$WASP_DIR=/opt/wasp` |

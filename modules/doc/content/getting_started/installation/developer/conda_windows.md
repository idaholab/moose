# Developer Install: Conda (Windows)

!alert note title=Feedback welcome
These instructions were recently updated. If anything is unclear or you have suggestions for improvement, please share your feedback in this [GitHub discussion](https://github.com/idaholab/moose/discussions/32966).

The following instructions describe the use of a pre-built [Conda](https://docs.conda.io) development environment on Windows to build and execute MOOSE and MOOSE-based applications. The Conda development environment does not contain pre-built versions of MFEM, libTorch, and NEML2. If you want to use these optional packages, you will need to also install those dependencies yourself.

## Usage id=usage

### Install WSL

!include installation/includes/install_wsl.md

### Install Miniforge

[Miniforge](https://github.com/conda-forge/miniforge) must be installed within the WSL instance to provide the [Conda](https://docs.conda.io) environment for installing packages. Miniforge only needs to be installed once. If you run into issues during these steps, please visit our [Conda Troubleshooting](help/faq/conda_issues.md optional=True) guide.

Open a new WSL ubuntu window. First, run the following:

```bash
curl -L -O https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-x86_64.sh
bash Miniforge3-Linux-x86_64.sh -b -p ~/miniforge
```

!include installation/developer/includes/install_miniforge.md

!include installation/developer/includes/update_uninstall_miniforge.md

# Conda Linux Development Environment

!alert note title=Feedback welcome
These instructions were recently updated. If anything is unclear or you have suggestions for improvement, please share your feedback in this [GitHub discussion](https://github.com/idaholab/moose/discussions/32966).

The following instructions describe the use of [Conda](https://docs.conda.io) to install the required dependencies for building MOOSE and a MOOSE-based application on Linux.

The Conda development environment does not contain installed versions of MFEM, libTorch, and NEML2. If you want to use these optional packages, you will need to also install those dependencies yourself.

!alert note title=Use Apptainer
The preferred method of building MOOSE and MOOSE-based applications on Linux is via [Apptainer](https://apptainer.org/). Please consider using the instructions in [installation/developer/apptainer.md] instead. The Conda development environment described here is not as reproducible as the Apptainer environment. Additionally, the Apptainer environment contains additional optional dependencies of MOOSE like MFEM, libTorch, and NEML2 that the Conda environment does not.

## Usage id=usage

### Install Miniforge

[Miniforge](https://github.com/conda-forge/miniforge) first must be installed to provide the [Conda](https://docs.conda.io) environment for installing packages. Miniforge only needs to be installed once. If you run into issues during these steps, please visit our [Conda Troubleshooting](help/faq/conda_issues.md optional=True) guide.

```bash
curl -L -O https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-x86_64.sh
bash Miniforge3-Linux-x86_64.sh -b -p ~/miniforge
```

!include installation/developer/includes/install_miniforge.md

!include installation/developer/includes/update_uninstall_miniforge.md

# Developer Install: Conda (Linux)

!alert note title=Feedback welcome
These instructions were recently updated. If anything is unclear or you have suggestions for improvement, please share your feedback in this [GitHub discussion](https://github.com/idaholab/moose/discussions/32966).

The following instructions describe the use of a pre-built [Conda](https://docs.conda.io) development environment on Linux to build and execute MOOSE and MOOSE-based applications. The Conda development environment does not contain pre-built versions of MFEM, libTorch, and NEML2. If you want to use these optional packages, you will need to also install those dependencies yourself.

!alert note title=Use Apptainer
The preferred method of building MOOSE and MOOSE-based applications on Linux is via [Apptainer](https://apptainer.org/). Please consider using the instructions in [installation/developer/apptainer.md] instead. The Conda development environment described here is not as reproducible as the Apptainer environment. Additionally, the Apptainer environment contains additional optional dependencies of MOOSE like MFEM, libTorch, and NEML2 that the Conda environment does not.

To begin, follow these instructions:

1. [#install_miniforge]: Install Miniforge to enable the creation of Conda environments.
1. [#create_environment]: Create the Conda environment that contains the required dependencies.
1. [#activate_environment]: Activate the created Conda environment that contains the required dependencies.
1. [#build_and_test]: Build and test an application.

After the environment has been created, you can use it again in another terminal window by repeating the instructions in [#activate_environment].

If you need to update the environment, follow the instructions in [#updating].

## Install Miniforge id=install_miniforge

[Miniforge](https://github.com/conda-forge/miniforge) first must be installed to provide the [Conda](https://docs.conda.io) environment for installing packages. Miniforge only needs to be installed once. If you run into issues during these steps, please visit our [Conda Troubleshooting](help/faq/conda_issues.md optional=True) guide.

```bash
curl -L -O https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-x86_64.sh
bash Miniforge3-Linux-x86_64.sh -b -p ~/miniforge
```

!include installation/developer/includes/install_miniforge.md

!include installation/developer/includes/update_uninstall_miniforge.md

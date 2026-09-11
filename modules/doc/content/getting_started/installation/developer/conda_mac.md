# Developer Install: Conda (Mac)

!alert note title=Feedback welcome
These instructions were recently updated. If anything is unclear or you have suggestions for improvement, please share your feedback in this [GitHub discussion](https://github.com/idaholab/moose/discussions/32966).

The following instructions describe the use of a pre-built [Conda](https://docs.conda.io) development environment on Mac OS to build and execute MOOSE and MOOSE-based applications. The Conda development environment does not contain pre-built versions of MFEM, libTorch, and NEML2. If you want to use these optional packages, you will need to also install those dependencies yourself.

To begin, follow these instructions:

1. [#install_miniforge]: Install Miniforge to enable the creation of Conda environments.
1. [#create_environment]: Create the Conda environment that contains the required dependencies.
1. [#activate_environment]: Activate the created Conda environment that contains the required dependencies.
1. [#build_and_test]: Build and test an application.

After the environment has been created, you can use it again in another terminal window by repeating the instructions in [#activate_environment].

If you need to update the environment, follow the instructions in [#updating].

## Install Miniforge id=install_miniforge

[Miniforge](https://github.com/conda-forge/miniforge) first must be installed to provide the [Conda](https://docs.conda.io) environment for installing packages. Miniforge only needs to be installed once. If you run into issues during these steps, please visit our [Conda Troubleshooting](help/faq/conda_issues.md optional=True) guide.

Follow the below instructions based on whether or not you have an ARM mac or an Intel Mac.

+Install on ARM Mac+

```bash
curl -L -O https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-arm64.sh
bash Miniforge3-MacOSX-arm64.sh -b -p ~/miniforge
```

+Install on Intel Mac+

```bash
curl -L -O https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-x86_64.sh
bash Miniforge3-MacOSX-arm64.sh -b -p ~/miniforge
```

!include installation/developer/includes/install_miniforge.md

!include installation/developer/includes/update_uninstall_miniforge.md

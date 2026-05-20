## Linux Install id=linux

The preferred method of installation of the development environment on Linux is using Apptainer.

- [installation/developer/apptainer.md] +(preferred)+

  - Use [Apptainer](https://apptainer.org/) to execute in development environment containers.
  - Contains optional dependencies (libTorch, MFEM, NEML2).

- [installation/developer/inl_hpc.md] +(preferred on INL HPC)+

  - Modules on INL HPC that are generated from the Apptainer environments listed above for easy access.
  - Contains optional dependencies (libTorch, MFEM, NEML2).

- [installation/developer/conda_linux.md]

  - Use [Conda](https://docs.conda.io) to distribute the development environment.
  - Does not contain optional dependencies (libTorch, MFEM, NEML2).

## Mac OS Install id=mac_os

Installation on Mac OS is only officially supported using Conda and Docker. We generally advise against the use of Homebrew on Mac OS. Intel Macs are currently still supported but this support will end at the end of 2026 as software updates to these machines are reduced to only security updates.

- [installation/developer/conda_mac.md] +(preferred)+

  - Use [Conda](https://docs.conda.io) to distribute the development environment.
  - Does not contain optional dependencies (libTorch, MFEM, NEML2).

## Windows Install id=windows

- [installation/developer/conda_windows.md] +(preferred)+

  - Use [Conda](https://docs.conda.io) to distribute the development environment within WSL (Windows Subsystem for Linux).
  - Does not contain optional dependencies (libTorch, MFEM, NEML2).
  - Support is limited.

# Conda MOOSE Environment

Our preferred method for obtaining libraries necessary for MOOSE based
Application development, is via Conda's myriad array of libraries. Follow these
instructions to create an environment on your machine using Conda. At this time,
an option to install MOOSE directly on a Windows system is not yet supported.
On-going efforts are being made to add a conda installation option for Windows,
and an experimental [WSL](installation/windows10.md) option is available.


!include sqa/minimum_requirements.md

## Prerequisites

!include installation/remove_moose_environment.md

!include installation/install_miniconda.md

!include installation/install_conda_moose.md

!include getting_started/installation/clone_moose.md

!include getting_started/installation/test_moose.md

Head back over to the [getting_started/index.md optional=True] page to continue your tour of MOOSE.

!include installation/uninstall_conda.md

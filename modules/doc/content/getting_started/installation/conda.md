# Conda MOOSE Environment

!style! halign=left
Our preferred method for obtaining dependencies necessary for MOOSE-based
application development is via Conda's myriad array of libraries. Follow these
instructions to create an environment on your machine using Conda.
!style-end!

!include getting_started/installation/install_miniconda.md

!include getting_started/installation/install_conda_moose.md

## Cloning MOOSE

!style! halign=left
!template load file=installation/clone_moose.md.template PATH=~/projects
!style-end!

## Build and Test MOOSE

!style! halign=left
!template load file=installation/build_moose.md.template PATH=~/projects
!style-end!

!template load file=installation/test_moose.md.template PATH=~/projects

!include getting_started/installation/uninstall_conda.md

!include installation/conda_pagination.md optional=True

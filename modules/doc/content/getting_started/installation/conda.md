# Conda MOOSE Environment

!style halign=left
Our preferred method for obtaining dependencies necessary for MOOSE-based application development is
via Conda's myriad array of available libraries. Follow these instructions to create an environment
on your machine using Conda.

!alert tip
Those interested in operating in their own optimized environment, please seek help from one of our
'[From Source](getting_started/installation/index.md optional=True)' instructional pages. Our Conda
packages are designed to 'just work', making them unsuitable for things like benchmark case studies.

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

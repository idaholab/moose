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

If your tests ran successfully, head back over to the
[getting_started/installation/index.md optional=True] page to continue your tour of MOOSE.

!include getting_started/installation/uninstall_conda.md

Now that you have a working MOOSE, and you know how to make your MPI wrapper available, proceed to
'New Users' to begin your tour of MOOSE!

!content pagination use_title=True
                    previous=installation/index.md
                    next=getting_started/new_users.md

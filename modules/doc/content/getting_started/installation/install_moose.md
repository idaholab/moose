# Obtaining and Building MOOSE

!template load file=installation/clone_moose.md.template PATH=~/projects

## Build PETSc and libMesh

!template load file=installation/build_petsc_and_libmesh.md.template PATH=~/projects

## Compile and Test MOOSE

!template load file=installation/test_moose.md.template PATH=~/projects

If the installation was successful you should see most of the tests passing (some tests will be
skipped depending on your system environment), and no failures.


Now that you have a working MOOSE, proceed to [New Users](getting_started/new_users.md). A
primer which will walk you through creating your MOOSE based application.

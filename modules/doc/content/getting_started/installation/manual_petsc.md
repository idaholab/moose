## Compile PETSc

By default, we use PETSc submodule inside of MOOSE as our nonlinear/linear solvers.
PETSc can be built using the following script:

```bash
cd ~/projects/moose

unset PETSC_DIR PETSC_ARCH
./scripts/update_and_rebuild_petsc.sh
```

!alert! note
If you prefer to install PETSc into a specified location, use the following:

```bash
cd ~/projects/moose

unset PETSC_DIR PETSC_ARCH
./scripts/update_and_rebuild_petsc.sh --prefix=/where/you/want/to/put/petsc
```

During the follow-up libMesh compile, you need to set `PETSC_DIR`, that is,

```bash
export PETSC_DIR=/where/you/want/to/put/petsc
```
!alert-end!

If PETSc is built successfully, you should see some output like the following:

!package! code max-height=400
Now to check if the libraries are working do:
make PETSC_DIR=/your/home/projects/moose/scripts/../petsc PETSC_ARCH=arch-moose check

!package-end!

You could optionally check if PETSc works as follows:

```bash
cd ./petsc

make PETSC_DIR=/your/home/projects/moose/scripts/../petsc PETSC_ARCH=arch-moose check
```

PETSc should produce the output like this:

!package! code max-height=400
Running check examples to verify correct installation
Using PETSC_DIR=/your/home/projects/moose/scripts/../petsc and PETSC_ARCH=arch-moose
C/C++ example src/snes/tutorials/ex19 run successfully with 1 MPI process
C/C++ example src/snes/tutorials/ex19 run successfully with 2 MPI processes
C/C++ example src/snes/tutorials/ex19 run successfully with hypre
C/C++ example src/snes/tutorials/ex19 run successfully with mumps
C/C++ example src/snes/tutorials/ex19 run successfully with superlu_dist
Completed test examples

!package-end!

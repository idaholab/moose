## How do I build and use my own PETSc? id=petsc-myown

PETSc is supplied by our Conda installation. However, if you wish to build and use your own PETSc installation, you can use our
`update_and_rebuild_petsc.sh` script.

By building your own PETSc, it will be necessary to build your own libMesh as well. It is advisable to operate in an entirely new Conda environment for the sake of keeping your environment sane:

```bash
conda create -n custom-petsc moose-mpich
conda activate custom-petsc
cd ~/projects/moose/scripts
./update_and_rebuild_petsc.sh
./update_and_rebuild_libmesh.sh
```

The above creates a new Conda environment: 'custom-petsc', and installs the needed MPICH dependency. We then activate it, build PETSc, and then libMesh.

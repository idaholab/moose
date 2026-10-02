## How do I build and use my own libMesh? id=libmesh-myown

libMesh is supplied by our Conda installation. However, if you wish to build and use your own
libMesh installation, you can use our `update_and_rebuild_libmesh.sh` script.

Due to having `moose-libmesh` installed, Conda creates and sets an influential environment
variable `LIBMESH_DIR` that you would have to continually `unset`. It is therefore advisable to
create a new Conda environment without `moose-libmesh` installed. Use `moose-build` for the MOOSE
compiler/MPI toolchain, `moose-petsc` for the PETSc dependency, and `moose-tools` for the MOOSE
Python/tooling dependencies:

```bash
conda create -n custom-libmesh moose-build moose-petsc moose-tools
conda activate custom-libmesh
cd ~/projects/moose/scripts
./update_and_rebuild_libmesh.sh
```

The above creates a new Conda environment named `custom-libmesh`, installs the build tools and
PETSc dependency needed to build your own libMesh, activates it, and then builds libMesh.

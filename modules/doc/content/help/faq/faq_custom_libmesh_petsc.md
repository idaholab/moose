# Using a Custom libMesh or PETSc

The build instructions given on [getting_started/installation/conda.md] give instructions
that utilize Conda packages that supply an installation of [libMesh] and [PETSc] so that
MOOSE users and developers do not need to compile these libraries themselves. However,
with some small changes to these instructions, developers can build and use their own
installations of these libraries.

## Custom libMesh

To use a custom libMesh, the following Conda environment can be created (which we name `custom-libmesh` for example):

```bash
conda create -n custom-libmesh moose-petsc moose-build
```

or if you want to compile with [VTK] support,

```bash
conda create -n custom-libmesh moose-libmesh-vtk moose-petsc moose-build
```

Then activate the new environment and run the libMesh installation script:

```bash
conda activate custom-libmesh
cd ~/projects/moose
./scripts/update_and_rebuild_libmesh.sh
```

If building your own VTK (beyond the scope of this document), supply the appropriate VTK installation
path to the install script:

```bash
./scripts/update_and_rebuild_libmesh.sh --with-vtk-lib=/path/to/vtk/lib --with-vtk-include=/path/to/vtk/include
```

## Custom PETSc

To use a custom PETSc (and libMesh), the following Conda environment can be created (which we name `custom-petsc` for example):

```bash
conda create -n custom-petsc moose-build
```

Then activate the new environment and run the PETSc and libMesh installation scripts:

```bash
conda activate custom-petsc
cd ~/projects/moose
./scripts/update_and_rebuild_petsc.sh
./scripts/update_and_rebuild_libmesh.sh
```

[VTK]: https://vtk.org

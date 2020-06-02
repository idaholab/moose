# Frequently Asked Questions

## Do you have a mailing list where I can ask questions?

- moose-users@googlegroups.com - Technical Q&A (moderate traffic)
- moose-announce@googlegroups.com - Announcements (very light traffic)
- You can also browse our [mailing list](https://groups.google.com/forum/#!forum/moose-users).

## I can not build my application

- Please see our [Build Issues](help/troubleshooting.md#buildissues) troubleshooting section.

## gethostbyname failed, localhost (errno 3) error when running tests

- Please see 'gethostbyname' failure in our [Failing Tests](help/troubleshooting.md#failingtests) troubleshooting section.

## Some/All of my tests fail

- Please see our [Failing Tests](help/troubleshooting.md#failingtests) troubleshooting section.

## How do I build and use my own libMesh? id=libmesh-myown

libMesh is supplied by our Conda installation. However, if you wish to build and use your own libMesh installation, you can use our
`update_and_rebuild_libmesh.sh` script.

Due to having moose-libmesh installed, Conda creates and sets an influential environment variable LIBMESH_DIR. One you would have to continually `unset`. It is therefor advisable to create a new Conda environment without moose-libmesh installed:

```bash
conda create --name custom-libmesh moose-petsc
conda activate custom-libmesh
cd ~/projects/moose/scripts
./update_and_rebuild_libmesh.sh
```

The above creates a new Conda environment: 'custom-libmesh', installs the needed PETSc dependency so we can build our own libMesh, activates it, and then builds libMesh.

## How do I compile libMesh with VTK? id=libmesh-vtk

[libMesh] by default is compiled with [VTK] when using our moose-libmesh Conda package. However, if you are attempting to build your own version of libMesh with VTK support, you will need to either; install moose-libmesh-vtk Conda package and re-run through the above 'How do I build and use my own libMesh' step, or build your own VTK (beyond the scope of this document), and supply the path to your newly built VTK, when running `update_and_rebuild_libmesh.sh`:

```bash
./update_and_rebuild_libmesh.sh --with-vtk-lib=/path/to/vtk/lib --with-vtk-include=/path/to/vtk/include
```

## How do I build and use my own PETSc? id=petsc-myown

PETSc is supplied by our Conda installation. However, if you wish to build and use your own PETSc installation, you can use our
`update_and_rebuild_petsc.sh` script.

By building your own PETSc, it will be necessary to build libMesh as well. And like libMesh (above), it is advisable to operate in an entirely new Conda environment for the sake of keeping your environment sane:

```bash
conda create --name custom-petsc moose-mpich
conda activate custom-petsc
cd ~/projects/moose/scripts
./update_and_rebuild_petsc.sh
./update_and_rebuild_libmesh.sh
```

The above creates a new Conda environment: 'custom-petsc', installs the needed MPICH dependency so we can build our own PETSc, activates it, builds PETSc, and then libMesh.

!alert note title=We just built libMesh
If following 'How do I build and use my own PETSc', know that during this step we also built libMesh. Making it unnecessary to also perform 'How do I build and use my own libMesh'. Performing both would negate the PETSc build step.


[libMesh]: http://libmesh.github.io/

[VTK]: https://vtk.org

## How do I compile libMesh with VTK? id=libmesh-vtk

[libMesh] by default is compiled with [VTK] when using our moose-libmesh Conda package. However, if you are attempting to build your own version of libMesh with VTK support, the easiest way to do so, is to install all the dependencies via Conda, and then run the `./update_and_rebuild_libmesh.sh` script:

```bash
conda create -n custom-libmesh moose-petsc moose-libmesh-vtk
conda activate custom-libmesh
cd ~/projects/moose/scripts
./update_and_rebuild_libmesh.sh
```

Or, if you wish to build your own [VTK] (beyond the scope of this document), you need to explain to libMesh where this new installation of VTK is by providing the lib and include paths to it:

```bash
conda create --name custom-libmesh moose-petsc
conda activate custom-libmesh
cd ~/projects/moose/scripts
./update_and_rebuild_libmesh.sh --with-vtk-lib=/path/to/vtk/lib --with-vtk-include=/path/to/vtk/include
```

[VTK]: https://vtk.org

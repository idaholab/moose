# Frequently Asked Questions

## Do you have a mailing list where I can ask questions?

- moose-users@googlegroups.com - Technical Q&A (moderate traffic)
- moose-announce@googlegroups.com - Announcements (very light traffic)
- You can also browse our [mailing list](https://groups.google.com/forum/#!forum/moose-users).

## I can not build my application

- Please see our [Build Issues](help/troubleshooting.md#buildissues) troubleshooting section.

## libMesh fails to compile

- Please see our [libMesh](help/troubleshooting.md#libmesh) troubleshooting section.

## gethostbyname failed, localhost (errno 3) error when running tests

- Please see 'gethostbyname' failure in our [Failing Tests](help/troubleshooting.md#failingtests) troubleshooting section.

## Some/All of my tests fail

- Please see our [Failing Tests](help/troubleshooting.md#failingtests) troubleshooting section.

## GCC not completly functional warning

- Please see our [MacOS Catalina Caveats](help/troubleshooting.md#catalinacaveats) section.

## How do I compile libMesh with VTK? id=libmesh-vtk

[libMesh] can optionally be compiled to include [VTK] by add the following command line arguments
to the build script.

```bash
cd ~/projects/moose/scripts
./update_and_rebuild_libmesh.sh --with-vtk-include=$VTKINCLUDE_DIR --with-vtk-lib=$VTKLIB_DIR
```
If you are using the legacy MOOSE environment package, the following modules should be loaded, which
will automatically set the correct environment variables for the above command.

```bash
module load advanced_modules
module load vtk-clang
```

For custom installations the `$VTKINCLUDE_DIR` and `$VTKLIB_DIR` will need to be set to the correct
locations for your installation of [VTK].

!alert note title=Conda installs libMesh with VTK by default
If you are using the conda install instructions [VTK] is enabled by default and this
step is not needed.


[libMesh]: http://libmesh.github.io/

[VTK]: https://vtk.org

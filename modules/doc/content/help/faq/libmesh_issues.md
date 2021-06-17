## Compiling libMesh id=libmesh

Compiling libMesh requires a proper environment. Lets verify a few things before attempting to build it (or possibly re-build it in your case):

- Verify you have a proper compiler present:

  - Linux:

    !package! code max-height=400
    which $CC
    /opt/moose/mpich-__MPICH__/gcc-__GCC__/bin/mpicc

    mpicc -show
    gcc -I/opt/moose/mpich-__MPICH__/gcc-__GCC__/include -L/opt/moose/mpich-__MPICH__/gcc-__GCC__/lib -Wl,-rpath -Wl,/opt/moose/mpich-__MPICH__/gcc-__GCC__/lib -Wl,--enable-new-dtags -lmpi

    which gcc
    /opt/moose/gcc-__GCC__/bin/gcc
    !package-end!

  - Macintosh:

    !package! code max-height=400
    which $CC
    /opt/moose/mpich-__MPICH__/clang-__LLVM__/bin/mpicc

    mpicc -show
    clang -Wl,-commons,use_dylibs -I/opt/moose/mpich-__MPICH__/clang-__LLVM__/include -L/opt/moose/mpich-__MPICH__/clang-__LLVM__/lib -lmpi -lpmpi

    which clang
    /opt/moose/llvm-__LLVM__/bin/clang
    !package-end!

  What you are looking for is that `which` and `mpicc -show` are returning proper paths. If these paths are not set, or `which` is not returning anything, see [Modules](help/troubleshooting.md#modules) for help on setting up a proper environment. Once set up, return here and verify the above commands return the proper messages.


- Check that PETSC_DIR is set and does exist:

  - Linux:

    !package! code max-height=400
    echo $PETSC_DIR
    /opt/moose/petsc-__PETSC_DEFAULT__/mpich-__MPICH___gcc-__GCC__-opt

    file $PETSC_DIR
    /opt/moose/petsc-__PETSC_DEFAULT__/mpich-__MPICH___gcc-__GCC__-opt: directory
    !package-end!

  - Macintosh:

    !package! code max-height=400
    echo $PETSC_DIR
    /opt/moose/petsc-__PETSC_DEFAULT__/mpich-__MPICH___clang-__LLVM__-opt

    file $PETSC_DIR
    /opt/moose/petsc-__PETSC_DEFAULT__/mpich-__MPICH___clang-__LLVM__-opt: directory
    !package-end!

  - If `echo $PETSC_DIR` returns nothing, this would indicate your environment is not complete. See [Modules](help/troubleshooting.md#modules) for help on setting up a proper environment. Once set up, return here and verify the above commands return the proper messages.

  - If `file $PETSC_DIR` returns an error (possible if you are performing a Manual Install), it would appear you have not yet ran configure. Configure builds this directory.


- With the above all taken care of, try to build libMesh:

  ```bash
  cd moose/scripts
  ./update_and_rebuild_libmesh.sh
  ```

  If you encounter errors during this step, we would like to hear from you! Please seek help on the [MOOSE Discussion forum](https://github.com/idaholab/moose/discussions). Provide the diagnostic and libmesh configure logs. Those two files can be found in the following locations:

  - `moose/libmesh/build/config.log`
  - `moose/scripts/libmesh_diagnostic.log`


- If libMesh built successfully, return to the beginning of the step that lead you here, and try that step again.

## Compiling libMesh id=libmesh

Compiling libMesh requires a proper environment. Lets verify a few things before attempting to build it (or possibly re-build it in your case):

- Verify you have a proper compiler present:

  - Linux:

    ```bash
    which $CC
    /opt/moose/mpich-3.2/gcc-7.3.1/bin/mpicc

    mpicc -show
    gcc -I/opt/moose/mpich-3.2/gcc-7.3.1/include -L/opt/moose/mpich-3.2/gcc-7.3.1/lib -Wl,-rpath -Wl,/opt/moose/mpich-3.2/gcc-7.3.1/lib -Wl,--enable-new-dtags -lmpi

    which gcc
    /opt/moose/gcc-7.3.1/bin/gcc
    ```

  - Macintosh:
  
    ```bash
    which $CC
    /opt/moose/mpich-3.2/clang-6.0.1/bin/mpicc

    mpicc -show
    clang -Wl,-commons,use_dylibs -I/opt/moose/mpich-3.2/clang-6.0.1/include -L/opt/moose/mpich-3.2/clang-6.0.1/lib -lmpi -lpmpi

    which clang
    /opt/moose/llvm-6.0.1/bin/clang
    ```
  
  What you are looking for is that `which` and `mpicc -show` are returning proper paths. If these paths are not set, or `which` is not returning anything, see [Modules](help/troubleshooting.md#modules) for help on setting up a proper environment. Once set up, return here and verify the above commands return the proper messages.


- Check that PETSC_DIR is set and does exist:

  - Linux:

    ```bash
    echo $PETSC_DIR
    /opt/moose/petsc-3.8.3/mpich-3.2_gcc-7.3.1-opt

    file $PETSC_DIR
    /opt/moose/petsc-3.8.3/mpich-3.2_gcc-7.3.1-opt: directory
    ```
    
  - Macintosh:

    ```bash
    echo $PETSC_DIR
    /opt/moose/petsc-3.8.3/mpich-3.2_clang-6.0.1-opt

    file $PETSC_DIR
    /opt/moose/petsc-3.8.3/mpich-3.2_clang-6.0.1-opt: directory
    ```

  - If `echo $PETSC_DIR` returns nothing, this would indicate your environment is not complete. See [Modules](help/troubleshooting.md#modules) for help on setting up a proper environment. Once set up, return here and verify the above commands return the proper messages.

  - If `file $PETSC_DIR` returns an error (possible if you are performing a Manual Install), it would appear you have not yet ran configure. Configure builds this directory.


- With the above all taken care of, try to build libMesh:

  ```bash
  cd moose/scripts
  ./update_and_rebuild_libmesh.sh
  ```

  If you encounter errors during this step, we would like to hear from you! Please seek help on our [mailing list](https://groups.google.com/forum/#!forum/moose-users). Provide the diagnostic and libmesh configure logs. Those two files can be found in the following locations:

  - `moose/libmesh/build/config.log`
  - `moose/scripts/libmesh_diagnostic.log`


- If libMesh built successfully, return to the beginning of the step that lead you here, and try that step again.

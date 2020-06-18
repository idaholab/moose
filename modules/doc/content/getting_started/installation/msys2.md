# MSYS2

Install MSYS2 following the instructions on the [MSYS2 homepage](https://www.msys2.org/).
Choose the 64bit version, keep the suggested install location (or you will have to adapt below).
Once the two `pacman` runs are completed make sure you open the `MINGW64` terminal
and use it for all further steps.

## prerequisites

### MS MPI and toolchain

Git, compilers, and MPI are installed using packages.

```
pacman -S base-devel binutils git mingw-w64-x86_64-toolchain
pacman -S mingw-w64-x86_64-msmpi
```

Next download and install `msmpisetup.exe` (not `msmpisdk.msi`!) from Microsoft.
Stick with the default installation path (else you will have to modify the `export` below).
Pick version that matches the msys2 msmpi sdk package version. As of the time of
writing this, it is MS MPI 10. Add the MS MPI runtime tools to your path (add this to `~/.bashrc`)

```
export PATH=$PATH:/c/Program\ Files/Microsoft\ MPI/Bin/
```

### Metis, Parmetis, Blas, netcdf

We will use MSYS2 packages for those PETSc dependencies as downloading and compiling
from the PETSc config script does not work (the MSYS2 packages includes some minor patches to
make everything work).

```
pacman -S mingw-w64-x86_64-metis mingw-w64-x86_64-parmetis mingw-w64-x86_64-openblas
```

### MOOSE repository

Create a `projects` directory and change into it

```
mkdir ~/projects
cd ~/projects
```

Clone moose and initialize its submodules

```
git clone https://github.com/idaholab/moose
cd moose
git submodule update --init --recursive
```

Add the `MOOSE_DIR` environment variable to your `~/.bashrc` config and reload the
bash configuration (make sure you are the `moose` directory when you do this!)

```
echo "export MOOSE_DIR='`pwd`'" >> ~/.bashrc
. ~/.bashrc
```

# PETSc

Next we build PETSc. We have prepared a special configuration script for PETSc
to make sure we use the preinstalled prerequisites and leave off optional features
that may cause the build to fail.

```
cd $MOOSE_DIR/petsc
../scripts/petsc_msys2_config.py
```

Follow the instructions printed at the end of the configure output on how to run make.

Once PETSc is built do **not** run the `make install` command. We'll leave the built
PETSc where it is and add two exports for the `PETSC_DIR` and `PETSC_ARCH` to your `~/.bashrc`.

**IMPORTANT:** make sure you prepend `C:/msys64` to the `PETSC_DIR`. The lines in your
`~/.bashrc` should look something like this:

```
export PETSC_DIR=C:/msys64/home/`whoami`/projects/moose/petsc
export PETSC_ARCH=petsc_msys2_config
```

Source your bash configuration to get the new environment variables and run

```
../scripts/petsc_msys2_fixup.py
```

This fixes some broken paths in `$PETSC_DIR/$PETSC_ARCH/lib/petsc/conf/petscvariables`

# libMesh

!alert note
libmesh requires a patch to enable using an external netcdf installation. That patch is not in the master branch or submodule yet.

Set the `LIBMESH_DIR` environment variable

- `export LIBMESH_DIR=$MOOSE_DIR/libmesh/installed` when using the libMesh MOOSE submodule
- `export LIBMESH_DIR=$HOME/projects/libmesh/installed` when using a separate libMesh repo (this assumes the repo is already checked out!)


Change into the libmesh repo dir (that's usually the parent of your `LIBMESH_DIR`)

```
mkdir -p $LIBMESH_DIR/installed
cd $LIBMESH_DIR/..
```

And run the libmesh script to fix missing symlinks

```
mkdir -p $LIBMESH_DIR/installed
cd $LIBMESH_DIR/..
./contrib/bin/fix_windows_symlinks.sh
```

Next we install netcdf from the MSYS2 repository (the version that is shipped with
libmesh won't work)

```
pacman -S mingw-w64-x86_64-netcdf
```

We will build the `opt` version only here. You can instead provide `METHODS='opt dbg'`
(note the `S`), but you may need to take extra measures building TMPI if you want to also
build the `oprof` method (i.e. passing the `--with-methods="..."` argument to TIMPI
configure).

Start the configure and build process with...

(continue in the appropriate subsection)

## libmesh (separate repo)

```
mkdir build
cd build
../configure --with-methods='opt' --prefix=$HOME/projects/libmesh/installed --with-static=yes --with-shared=no --enable-static --disable-shared --enable-unique-id --disable-maintainer-mode --enable-petsc-hypre-required --enable-metaphysicl-required --enable-netcdf-required NETCDF_DIR=C:/msys64/mingw64
make && make install
```

## libMesh (MOOSE submodule)

```
cd $MOOSE_DIR
METHOD=opt scripts/update_and_rebuild_libmesh.sh --enable-static --disable-shared --enable-netcdf-required NETCDF_DIR=C:/msys64/mingw64
```

This will take a few minutes.

# MOOSE

Now it is finally time to build MOOSE. We'll start with the framework and the
tests. (adjust the `-j` option to you number of available cores)

```
cd $MOOSE_DIR/test
make -j 2
```

To run the resulting executables from the native Windows shell you may need to
(for now) copy a few DLLs to the current directory. (this is not necessary when
running in the msys2 shell)

```
cp `ldd moose_test-opt.exe |grep mingw | cut -d\> -f2|cut -d' ' -f2` .
```

Unfortunately the final moose executable also needs to be linked by hand. First
install pcre

```
pacman -S mingw-w64-x86_64-pcre
```

Then get the proper linking options for pcrecpp

```
pkg-config libpcrecpp --libs --static
```

and add this to the end of the final linking line (check `make -n`) and remove
the contrib pcre stuff.

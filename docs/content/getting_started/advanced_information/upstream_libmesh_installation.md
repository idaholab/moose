Normally it is sufficient to utilize the version of libMesh that ships with MOOSE.  However, if we are working directly with you and add a new feature to libMesh specifically for you then we may ask you to utilize a newer version of libMesh directly before we ship it as the official version with MOOSE.

Use the following direction to do just that....

1) First grab libmesh, either by cloning it:
```bash
git clone https://github.com/libMesh/libmesh.git
```

2) cd to the clone directory and run:
```bash
export LIBMESH_DIR=`pwd`/installed
export METHODS=opt
mkdir build
cd build

../configure --with-methods="${METHODS}" \
             --prefix=$LIBMESH_DIR \
             --enable-default-comm-world \
             --enable-silent-rules \
             --enable-unique-id \
             --disable-warnings \
             --enable-openmp 

make -j $MOOSE_JOBS install
```
This assumes that `$MOOSE_JOBS` is set in your environment, replace with `-j N` where `N` is the number of processors you'd like to use for compiling, if `$MOOSE_JOBS` is not set for some reason.

3) Important: You are now responsible for setting `LIBMESH_DIR` in your environment *every* time you log in.  Some people like to control this with modules, other people just add
```bash
export LIBMESH_DIR=/path/to/your/libmesh
```
to their .bashrc_local file.  It's up to you.  If you forget to do it, MOOSE will pick up the version of libMesh in the MOOSE repository rather than the upstream libMesh you just built.

4) Once this part is done, you should be able to go to your MOOSE checkout, run make clean, and then rebuild it against your custom version of libMesh.  

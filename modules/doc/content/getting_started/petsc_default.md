```bash
./configure \
--prefix=$PETSC_DIR \
--download-hypre=1 \
--with-debugging=no \
--with-shared-libraries=1 \
--download-fblaslapack=1 \
--download-metis=1 \
--download-parmetis=1 \
--download-superlu_dist=1 \
--download-scalapack=1 \
--download-mumps=1 \
--download-slepc \
--with-mpi=1 \
--with-cxx-dialect=C++11 \
--with-fortran-bindings=0 \
--with-sowing=0 \
PETSC_DIR=`pwd`
```

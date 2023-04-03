## GCC

!style! halign=left
We need a modern C++17 capable compiler. Our minimum requirements are: GCC [!package!minimum_gcc],
Clang [!package!minimum_clang]. This section will focus on building a GCC [!package!gcc] compiler
stack.
!style-end!

Obtain GCC source:

!package! code
curl -L -O http://mirrors.concertpass.com/gcc/releases/gcc-__GCC__/gcc-__GCC__.tar.gz
tar -xf gcc-__GCC__.tar.gz -C .
!package-end!

Obtain GCC pre-reqs:

!package! code max-height=450
cd gcc-__GCC__
./contrib/download_prerequisites
!package-end!

Configure GCC using the recommended arguments:

!package! code
mkdir gcc-build
cd gcc-build

../configure --prefix=/target/installation/path/gcc-__GCC__ \
--disable-multilib \
--enable-languages=c,c++,fortran,jit \
--enable-checking=release \
--enable-host-shared \
--with-pic
!package-end!

With `configure` complete (and error free), build and install GCC:

```bash
make -j 6
make install
```

Follow the onscreen instructions on how to make use of your new compiler.

## GCC

We need a modern C++17 capable compiler. Our minimum requirements are: GCC [!package!minimum_gcc],
Clang [!package!minimum_clang]. This section will focus on building a GCC [!package!gcc] compiler
stack.

What version of GCC do we have?

```bash
gcc --version

gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-4)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

If your version is less than [!package!minimum_gcc], you will need to build a newer version. If your
version is at or greater than [!package!minimum_gcc], you have the option of skipping to MPICH

!package! code
cd $STACK_SRC
curl -L -O http://mirrors.concertpass.com/gcc/releases/gcc-__GCC__/gcc-__GCC__.tar.gz
tar -xf gcc-__GCC__.tar.gz -C .
!package-end!

Obtain GCC pre-reqs:

!package! code max-height=450
cd $STACK_SRC/gcc-__GCC__
./contrib/download_prerequisites
!package-end!

Configure, build and install GCC:

!package! code
mkdir $STACK_SRC/gcc-build
cd $STACK_SRC/gcc-build

../gcc-__GCC__/configure --prefix=$PACKAGES_DIR/gcc-__GCC__ \
--disable-multilib \
--enable-languages=c,c++,fortran,jit \
--enable-checking=release \
--enable-host-shared \
--with-pic

make -j #   (where # is the number of cores available)

make install -j # (where # is the number of cores available)
!package-end!

Any errors during configure/make will need to be investigated on your own. Every operating system I
have come across has its own nuances of getting stuff built. Normally any issues are going to be
solved by installing the necessary development libraries using your system package manager (apt-get,
yum, zypper, etc). Hint: I would search the internet for 'how to build GCC [!package!gcc] on (insert
the name/version of your operating system here)'

!alert! note
In order to utilize our newly built GCC [!package!gcc] compiler, we need to set some variables:

!package! code
export PATH=$PACKAGES_DIR/gcc-__GCC__/bin:$PATH
export LD_LIBRARY_PATH=$PACKAGES_DIR/gcc-__GCC__/lib64:$PACKAGES_DIR/gcc-__GCC__/lib:$PACKAGES_DIR/gcc-__GCC__/lib/gcc/x86_64-pc-linux-gnu/__GCC__:$PACKAGES_DIR/gcc-__GCC__/libexec/gcc/x86_64-pc-linux-gnu/__GCC__:$LD_LIBRARY_PATH
!package-end!

!alert-end!

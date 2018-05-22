## GCC

We need a modern C++11 capable compiler. Our minimum requirements are: GCC 4.8.4, Clang 3.4.0,
and Intel20130607. This section will focus on building a GCC 7.3.0 compiler stack.

What version of GCC do we have?

```bash
gcc --version

gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-4)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

If your version is less than 4.8.4, you will need to build a newer version. If your version is at or
greater than 4.8.4, you have the option of skipping the GCC section.

```bash
cd $STACK_SRC
curl -L -O http://mirrors.concertpass.com/gcc/releases/gcc-7.3.0/gcc-7.3.0.tar.gz
tar -xf gcc-7.3.0.tar.gz -C .
```

Obtain GCC pre-reqs:

```bash
cd $STACK_SRC/gcc-7.3.0
./contrib/download_prerequisites
```

Configure, build and install GCC:

```bash
mkdir $STACK_SRC/gcc-build
cd $STACK_SRC/gcc-build

../gcc-7.3.0/configure --prefix=$PACKAGES_DIR/gcc-7.3.0 \
--disable-multilib \
--enable-languages=c,c++,fortran,jit \
--enable-checking=release \
--enable-host-shared \
--with-pic

make -j #   (where # is the number of cores available)

make install
```

Any errors during configure/make will need to be investigated on your own. Every operating system I
have come across has its own nuances of getting stuff built. Normally any issues are going to be
solved by installing the necessary development libraries using your system package manager (apt-get,
yum, zypper, etc). Hint: I would search the internet for 'how to build GCC 5.4.0 on (insert the
name/version of your operating system here)'

!alert! note
In order to utilize our newly built GCC 7.3.0 compiler, we need to set some variables:

```bash
export PATH=$PACKAGES_DIR/gcc-7.3.0/bin:$PATH
export LD_LIBRARY_PATH=$PACKAGES_DIR/gcc-7.3.0/lib64:$PACKAGES_DIR/gcc-7.3.0/lib:$PACKAGES_DIR/gcc-7.3.0/lib/gcc/x86_64-unknown-linux-gnu/7.3.0:$PACKAGES_DIR/gcc-7.3.0/libexec/gcc/x86_64-unknown-linux-gnu/7.3.0:$LD_LIBRARY_PATH
```
!alert-end!

## LLVM/Clang

We will clone all the necessary repositories involved with building LLVM/Clang from source:

```bash
mkdir -p $STACK_SRC/llvm-src
cd $STACK_SRC/llvm-src
git clone https://github.com/llvm-mirror/llvm.git
git clone https://github.com/llvm-mirror/clang.git $STACK_SRC/llvm-src/llvm/tools/clang
git clone https://github.com/llvm-mirror/compiler-rt.git $STACK_SRC/llvm-src/llvm/projects/compiler-rt
git clone https://github.com/llvm-mirror/libcxx.git $STACK_SRC/llvm-src/llvm/projects/libcxx
git clone https://github.com/llvm-mirror/libcxxabi.git $STACK_SRC/llvm-src/llvm/projects/libcxxabi
git clone https://github.com/llvm-mirror/openmp.git $STACK_SRC/llvm-src/llvm/projects/openmp
git clone https://github.com/llvm-mirror/clang-tools-extra.git $STACK_SRC/llvm-src/llvm/tools/clang/tools/extra

cd $STACK_SRC/llvm-src/llvm
git checkout release_50
cd $STACK_SRC/llvm-src/llvm/tools/clang
git checkout release_50
cd $STACK_SRC/llvm-src/llvm/projects/compiler-rt
git checkout release_50
cd $STACK_SRC/llvm-src/llvm/projects/libcxx
git checkout release_50
cd $STACK_SRC/llvm-src/llvm/projects/libcxxabi
git checkout release_50
cd $STACK_SRC/llvm-src/llvm/projects/openmp
git checkout release_50
cd $STACK_SRC/llvm-src/llvm/tools/clang/tools/extra
git checkout release_50
```

And now we configure, build, and install Clang:

```bash
mkdir -p $STACK_SRC/llvm-src/build
cd $STACK_SRC/llvm-src/build
cmake -DCMAKE_INSTALL_RPATH:STRING=$PACKAGES_DIR/llvm-5.0.1/lib \
-DCMAKE_CXX_LINK_FLAGS="-L$PACKAGES_DIR/gcc-7.3.0/lib64 -Wl,-rpath,$PACKAGES_DIR/gcc-7.3.0/lib64" \
-DGCC_INSTALL_PREFIX=$PACKAGES_DIR/gcc-7.3.0 \
-DCMAKE_CXX_COMPILER=$PACKAGES_DIR/gcc-7.3.0/bin/g++ \
-DCMAKE_C_COMPILER=$PACKAGES_DIR/gcc-7.3.0/bin/gcc \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX=$PACKAGES_DIR/llvm-5.0.1 \
-DCMAKE_INSTALL_NAME_DIR:STRING=$PACKAGES_DIR/llvm-5.0.1/lib \
-DCMAKE_MACOSX_RPATH:BOOL=OFF \
-DPYTHON_EXECUTABLE=`which python2.7` \
-DLLVM_TARGETS_TO_BUILD="X86" \
-G 'Unix Makefiles' ../llvm

make -j # (where # is the number of cores available)

make install
```

!alert! note
In order to utilize our newly built LLVM-Clang compiler, we need to set some variables:

```bash
export PATH=$PACKAGES_DIR/llvm-5.0.1/bin:$PATH
export LD_LIBRARY_PATH=$PACKAGES_DIR/llvm-5.0.1/lib:$LD_LIBRARY_PATH
```
!alert-end!

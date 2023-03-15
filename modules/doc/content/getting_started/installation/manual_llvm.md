## LLVM/Clang

We will clone all the necessary repositories involved with building LLVM/Clang from source:

!package! code max-height=500
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
git checkout release___LLVM_RELEASE__
cd $STACK_SRC/llvm-src/llvm/tools/clang
git checkout release___LLVM_RELEASE__
cd $STACK_SRC/llvm-src/llvm/projects/compiler-rt
git checkout release___LLVM_RELEASE__
cd $STACK_SRC/llvm-src/llvm/projects/libcxx
git checkout release___LLVM_RELEASE__
cd $STACK_SRC/llvm-src/llvm/projects/libcxxabi
git checkout release___LLVM_RELEASE__
cd $STACK_SRC/llvm-src/llvm/projects/openmp
git checkout release___LLVM_RELEASE__
cd $STACK_SRC/llvm-src/llvm/tools/clang/tools/extra
git checkout release___LLVM_RELEASE__
!package-end!

And now we configure, build, and install Clang:

!package! code
mkdir -p $STACK_SRC/llvm-src/build
cd $STACK_SRC/llvm-src/build
cmake ../llvm -G 'Unix Makefiles' \
-DCMAKE_INSTALL_PREFIX=$PACKAGES_DIR/llvm-__LLVM__ \
-DCMAKE_INSTALL_RPATH:STRING=$PACKAGES_DIR/llvm-__LLVM__/lib \
-DCMAKE_INSTALL_NAME_DIR:STRING=$PACKAGES_DIR/llvm-__LLVM__/lib \
-DCMAKE_BUILD_WITH_INSTALL_RPATH=1 \
-DLLVM_TARGETS_TO_BUILD="X86" \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_MACOSX_RPATH:BOOL=OFF \
-DPYTHON_EXECUTABLE=`which python3` \
-DCMAKE_CXX_LINK_FLAGS="-L$PACKAGES_DIR/gcc-__GCC__/lib64 -Wl,-rpath,$PACKAGES_DIR/gcc-__GCC__/lib64" \
-DGCC_INSTALL_PREFIX=$PACKAGES_DIR/gcc-__GCC__ \
-DCMAKE_CXX_COMPILER=$PACKAGES_DIR/gcc-__GCC__/bin/g++ \
-DCMAKE_C_COMPILER=$PACKAGES_DIR/gcc-__GCC__/bin/gcc

make -j # (where # is the number of cores available)

make install
!package-end!

!alert! note
The above configuration assumes you are using the custom version of GCC built in the previous
section (note the several gcc-[!package!gcc] paths). If this is not the case, you will need to
provide the correct paths to your current toolchain. It is also possible LLVM may build successfully
if you omit the -D lines referencing gcc-[!package!gcc] entirely.
!alert-end!

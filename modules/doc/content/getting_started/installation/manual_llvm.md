## LLVM/Clang

Clone the LLVM-Project:

!package! code max-height=500
git clone https://github.com/llvm/llvm-project
cd llvm-project
git checkout llvmorg-__CLANG__
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

## LLVM Clang

!style! halign=left
Clone the LLVM-Project:
!style-end!

!package! code max-height=500
git clone https://github.com/llvm/llvm-project
cd llvm-project
git checkout llvmorg-__CLANG__
!package-end!

Configure LLVM using our recommended arguments, in addition to any optimizations you require:

!package! code
mkdir llvm-build
cd llvm-build
cmake ../llvm -G 'Unix Makefiles' \
-DLLVM_ENABLE_PROJECTS='clang;clang-tools-extra;compiler-rt;libcxx;libcxxabi;libunwind;openmp;lldb' \
-DCMAKE_INSTALL_PREFIX=/target/installation/path/llvm-__CLANG__ \
-DCMAKE_INSTALL_RPATH:STRING=/target/installation/path/llvm-__CLANG__/lib \
-DCMAKE_INSTALL_NAME_DIR:STRING=/target/installation/path/llvm-__CLANG__/lib \
-DCMAKE_BUILD_WITH_INSTALL_RPATH=1 \
-DLLVM_TARGETS_TO_BUILD="X86" \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_MACOSX_RPATH:BOOL=OFF \
-DCMAKE_CXX_LINK_FLAGS="-L/some/path/to/gcc-__GCC__/lib64 -Wl,-rpath,/some/path/to/gcc-__GCC__/lib64" \
-DGCC_INSTALL_PREFIX=/some/path/to/gcc-__GCC__ \
-DCMAKE_CXX_COMPILER=/some/path/to/gcc-__GCC__/bin/g++ \
-DCMAKE_C_COMPILER=/some/path/to/gcc-__GCC__/bin/gcc
!package-end!

!alert! note title=GCC Paths
The above configuration assumes you are using a custom version of GCC (note the several
gcc-[!package!gcc] paths)
!alert-end!

With `configure` complete (and error free), build and install LLVM Clang:

```bash
make -j 6
make install
```

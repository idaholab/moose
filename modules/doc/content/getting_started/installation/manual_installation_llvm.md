# LLVM Clang from Source

!alert! note title=Are you sure!? Are you sure!? Are you sure!?
Building an LLVM (Clang) compiler is substantially more difficult than GCC
!alert-end!

The following instructions are for those wishing to build their own LLVM compiler stack, when
optimizations are of paramount importance, or when performing benchmarks are the order of the day.

Because we need MPICH to ultimately wrap to a Fortran compiler, it will be necessary to build GCC
first, and then LLVM Clang. Therefore both compilers must be built *perfectly* (+you *really* need
to pay attention to any warnings/errors you encounter+) to achieve a proper LLVM Clang compiler
stack.

Due to the general nature of the following document, the commands you see here are for reference and
should not blindly be 'cut and pasted' into your terminal. You will certainly want to do some
research about your own system before begining (Intel vs AMD vs Apple Si, CPUs etc). You will want
to know what optimization switches work best for the given CPU on the machine you intend to run
MOOSE on.

## Prerequisites

- A *modern* GCC compiler with Fortran language enabled

!include manual_prereqs.md

!include manual_llvm.md

!include manual_mpich_llvm.md


With the compiler stack ready, you can proceed to the next section:

!content pagination use_title=True
                    next=installation/llvm_install_moose.md

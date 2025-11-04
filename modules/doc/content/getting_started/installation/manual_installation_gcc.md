# GCC from Source

The following instructions are for those wishing to build their own GCC compiler stack, when
optimizations are of paramount importance, or when performing benchmarks are the order of the day.

Due to the general nature of the following document, the commands you see here are for reference and
should not blindly be 'cut and pasted' into your terminal. You will certainly want to do some
research about your own system before begining (Intel vs AMD vs Apple Si, CPUs etc). You will want
to know what optimization switches work best for the given CPU on the machine you intend to run
MOOSE on.

## Prerequisites

!include manual_prereqs.md

!include manual_gcc.md

!include manual_mpich_gcc.md

With the compiler stack ready, you can proceed to the next section:

!content pagination use_title=True
                    next=installation/gcc_install_moose.md

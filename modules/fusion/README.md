FussionApp
=====

It is a moose-based application that temporality holds some
fusion thermal-hydraulics research files. These files may
be generalized, improved, and then moved to the MOOSE framework in the future.

At this point, the application holder is an entry point for
thermal-hydraulics simulations. Before using this application,
we need to make MOOSE ready. Here is detailed explanation of how
to build MOOSE https://mooseframework.inl.gov/getting_started/installation/index.html#install-moose

Submodule moose should be used since it has a few small necessary modifications.
Submodule moose should be used since it has a few small necessary modifications.
If you have a fresh clone, you could do "git submodule update --init" to prepare
all submodules.  If PETSc/libMesh is already, you could do "make -j20" compile
the application otherwise, please follow instructions
 https://mooseframework.inl.gov/getting_started/installation/index.html#install-moose
to have these packages ready. 

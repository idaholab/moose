<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# PFFracCoupledInterface
!description /Kernels/PFFracCoupledInterface
This kernel solves the laplacian operator of damage parameter c, and gets its residual.

!parameters /Kernels/PFFracCoupledInterface

{\bf c} Damage variable continuous between 0 and 1, 0 represents no damage, 1 represents fully cracked
{\bf beta} Auxiliary variable, which is the laplacian operator of c

!inputfiles /Kernels/PFFracCoupledInterface

!childobjects /Kernels/PFFracCoupledInterface

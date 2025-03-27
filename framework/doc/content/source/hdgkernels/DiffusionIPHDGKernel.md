# DiffusionIPHDGKernel

This kernel implements the element volume and interior face integrals of a diffusion term for a hybridized interior penalty (IP-H) discontinuous Galerkin discretization. The IP-H discretization for the diffusion/elliptic operator is described in [!citep](cockburn2009unified) which also details the hybridized local discontinuous Galerkin (LDG-H) discretization of an elliptic operator that is implemented in [DiffusionLHDGKernel.md].

!syntax parameters /HDGKernels/DiffusionIPHDGKernel

!syntax inputs /HDGKernels/DiffusionIPHDGKernel

!syntax children /HDGKernels/DiffusionIPHDGKernel


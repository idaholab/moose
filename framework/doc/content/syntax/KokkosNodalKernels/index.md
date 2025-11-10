# Kokkos NodalKernels System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [NodalKernels System](syntax/NodalKernels/index.md) to understand the MOOSE nodal kernel system,
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE,
- [Kokkos Kernels System](syntax/KokkosKernels/index.md) to understand the common design pattern of objects in Kokkos-MOOSE,
- [Kokkos BCs System](syntax/KokkosBCs/index.md) to understand the design pattern of nodal boundary conditions in Kokkos-MOOSE.

You can create your own nodal kernels by inheriting `Moose::Kokkos::NodalKernel` and following the same pattern with kernels and boundary conditions including registering with either `registerKokkosNodalKernel()` or `registerKokkosResidualObject()`.
The interfaces of nodal kernels are identical to the nodal boundary conditions described in [Kokkos BCs System](syntax/KokkosBCs/index.md), so they will not be explained here in detail.
See the following source codes of `KokkosCoupledForceNodalKernel` for an example of a nodal kernel:

!listing framework/include/kokkos/nodalkernels/KokkosCoupledForceNodalKernel.h id=kokkos-force-nodal-header
         caption=The `KokkosCoupledForceNodalKernel` header file.

!listing framework/src/kokkos/nodalkernels/KokkosCoupledForceNodalKernel.K id=kokkos-force-nodal-source language=cpp
         caption=The `KokkosCoupledForceNodalKernel` source file.

!syntax list /NodalKernels objects=True actions=False subsystems=False

!if-end!

!else
!include kokkos/kokkos_warning.md

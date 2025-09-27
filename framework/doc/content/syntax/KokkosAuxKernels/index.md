# Kokkos AuxKernels System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [AuxKernels System](syntax/AuxKernels/index.md) to understand the MOOSE kernel system,
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE,
- [Kokkos Kernels System](syntax/KokkosKernels/index.md) to understand the common design pattern of objects in Kokkos-MOOSE.

!syntax list /KokkosAuxKernels objects=True actions=False subsystems=False

!if-end!

!else
!include kokkos/kokkos_warning.md

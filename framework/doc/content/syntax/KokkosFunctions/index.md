# Kokkos Functions System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [Functions System](syntax/Functions/index.md) to understand the MOOSE function system,
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE,
- [Kokkos Kernels System](syntax/KokkosKernels/index.md) to understand the common design pattern of objects in Kokkos-MOOSE.

!syntax list /KokkosFunctions objects=True actions=False subsystems=False

!if-end!

!else
!include kokkos/kokkos_warning.md

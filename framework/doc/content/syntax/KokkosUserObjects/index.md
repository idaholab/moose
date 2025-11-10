# Kokkos UserObjects System

!if! function=hasCapability('kokkos')

Before reading this documentation, consider reading the following materials first for a better understanding of this documentation:

- [UserObjects System](syntax/UserObjects/index.md) to understand the MOOSE user object system,
- [Postprocessors System](syntax/Postprocessors/index.md) to understand the MOOSE postprocessor system,
- [VectorPostprocessors System](syntax/VectorPostprocessors/index.md) to understand the MOOSE vector postprocessor system,
- [Reporters System](syntax/Reporters/index.md) to understand the MOOSE reporter system.
- [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) to understand the programming practices for Kokkos-MOOSE,
- [Kokkos Kernels System](syntax/KokkosKernels/index.md) to understand the common design pattern of objects in Kokkos-MOOSE.

## Reducers id=reducers

!if-end!

!else
!include kokkos/kokkos_warning.md

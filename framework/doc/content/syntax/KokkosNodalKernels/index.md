# Kokkos NodalKernels System

!if! function=hasCapability('kokkos')

!alert note
Please read [NodalKernels System](syntax/Materials/index.md) and [Getting Started with Kokkos-MOOSE](syntax/Kokkos/index.md) first to have a basic understanding of the design of the MOOSE `NodalKernels` system and Kokkos-MOOSE.

!if-end!

!else
!include kokkos/kokkos_warning.md

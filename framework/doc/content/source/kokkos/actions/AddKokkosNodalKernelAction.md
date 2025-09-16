# AddKokkosNodalKernelAction

!if! function=hasCapability('kokkos')

!syntax description /KokkosNodalKernels/AddKokkosNodalKernelAction

Kokkos `NodalKernels` are specified as an object inside the `[KokkosNodalKernels]` block. This action adds them to the [Problem](syntax/Problem/index.md).

More information about Kokkos `NodalKernels` may be found on the [Kokkos NodalKernels syntax documentation](syntax/KokkosNodalKernels/index.md).

!syntax parameters /KokkosNodalKernels/AddKokkosNodalKernelAction

!if-end!

!else
!include kokkos/kokkos_warning.md

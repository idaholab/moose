# AddKokkosKokkosKernelAction

!if! function=hasCapability('kokkos')

!syntax description /KokkosKernels/AddKokkosKernelAction

Kokkos `Kernels` are specified as an object inside the `[KokkosKernels]` block. This action adds them to the [Problem](syntax/Problem/index.md).

More information about Kokkos `Kernels` can be found on the [Kokkos Kernels syntax documentation](syntax/KokkosKernels/index.md).

!syntax parameters /KokkosKernels/AddKokkosKernelAction

!if-end!

!else
!include kokkos/kokkos_warning.md

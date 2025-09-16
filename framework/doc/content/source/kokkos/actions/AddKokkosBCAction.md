# AddKokkosBCAction

!if! function=hasCapability('kokkos')

!syntax description /KokkosBCs/AddKokkosBCAction

Kokkos `BoundaryConditions` are specified as an object inside the `[KokkosBCs]` block. This action adds them to the [Problem](syntax/Problem/index.md).

More information about Kokkos `BoundaryConditions` can be found on the [Kokkos BoundaryConditions syntax documentation](syntax/KokkosBCs/index.md).

!syntax parameters /KokkosBCs/AddKokkosBCAction

!if-end!

!else
!include kokkos/kokkos_warning.md

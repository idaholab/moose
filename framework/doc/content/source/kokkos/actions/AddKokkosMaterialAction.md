# AddKokkosMaterialAction

!if! function=hasCapability('kokkos')

!syntax description /KokkosMaterials/AddKokkosMaterialAction

Kokkos `Materials` are specified as an object inside the `[KokkosMaterials]` block. This action adds them to the [Problem](syntax/Problem/index.md).

More information about Kokkos `Materials` and their parameters can be found on the [Kokkos Materials syntax documentation](syntax/KokkosMaterials/index.md).

!syntax parameters /KokkosMaterials/AddKokkosMaterialAction

!if-end!

!else
!include kokkos/kokkos_warning.md

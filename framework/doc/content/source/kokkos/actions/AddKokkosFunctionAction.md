# AddKokkosFunctionAction

!if! function=hasCapability('kokkos')

!syntax description /KokkosFunctions/AddKokkosFunctionAction

Kokkos `Functions` are specified as an object inside the `[KokkosFunctions]` block. This action adds them to the [Problem](syntax/Problem/index.md).

More information about Kokkos `Functions` can be found on the [Kokkos Functions syntax documentation](syntax/KokkosFunctions/index.md).

!syntax parameters /KokkosFunctions/AddKokkosFunctionAction

!if-end!

!else
!include kokkos/kokkos_warning.md

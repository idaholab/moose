# KokkosSelfShadowSideUserObject

!if! function=hasCapability('kokkos')

This is the Kokkos version of [SelfShadowSideUserObject](SelfShadowSideUserObject.md). See the original document for details.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_directional_flux_2d.i start=[shadow] end=[] include-end=true

!syntax parameters /UserObjects/KokkosSelfShadowSideUserObject

!syntax inputs /UserObjects/KokkosSelfShadowSideUserObject

!syntax children /UserObjects/KokkosSelfShadowSideUserObject

!if-end!

!else
!include kokkos/kokkos_warning.md

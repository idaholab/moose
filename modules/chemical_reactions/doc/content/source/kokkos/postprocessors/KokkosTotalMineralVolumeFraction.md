# KokkosTotalMineralVolumeFraction

!if! function=hasCapability('kokkos')

This is the Kokkos version of [TotalMineralVolumeFraction](TotalMineralVolumeFraction.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_mineral_volume_fraction.i start=[volume_frac] end=[] include-end=true

!syntax parameters /Postprocessors/KokkosTotalMineralVolumeFraction

!syntax inputs /Postprocessors/KokkosTotalMineralVolumeFraction

!syntax children /Postprocessors/KokkosTotalMineralVolumeFraction

!if-end!

!else
!include kokkos/kokkos_warning.md

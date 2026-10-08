# KokkosJouleHeatingHeatGeneratedAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [JouleHeatingHeatGeneratedAux](JouleHeatingHeatGeneratedAux.md). See the original document for details.

!alert note
The Kokkos version only takes the Joule heating from [!param](/AuxKernels/KokkosJouleHeatingHeatGeneratedAux/heating_term), and the deprecated option of computing it from a coupled electrostatic potential through [!param](/AuxKernels/JouleHeatingHeatGeneratedAux/elec) is not available. The heating term is a non-AD material property.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/kokkos_heat_source_materials.i start=[joule_heating] end=[] include-end=true

!syntax parameters /AuxKernels/KokkosJouleHeatingHeatGeneratedAux

!syntax inputs /AuxKernels/KokkosJouleHeatingHeatGeneratedAux

!syntax children /AuxKernels/KokkosJouleHeatingHeatGeneratedAux

!if-end!

!else
!include kokkos/kokkos_warning.md

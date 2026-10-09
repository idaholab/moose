# KokkosKineticDisPreRateAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [KineticDisPreRateAux](KineticDisPreRateAux.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_kinetic_rate.i start=[kinetic_rate0] end=[] include-end=true

!syntax parameters /AuxKernels/KokkosKineticDisPreRateAux

!syntax inputs /AuxKernels/KokkosKineticDisPreRateAux

!syntax children /AuxKernels/KokkosKineticDisPreRateAux

!if-end!

!else
!include kokkos/kokkos_warning.md

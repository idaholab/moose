# KokkosKineticDisPreConcAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [KineticDisPreConcAux](KineticDisPreConcAux.md). See the original document for details.

## Example Input Syntax

!listing chemical_reactions/test/tests/kokkos/kokkos_kinetic_rate.i start=[mineral0_conc] end=[] include-end=true

!syntax parameters /AuxKernels/KokkosKineticDisPreConcAux

!syntax inputs /AuxKernels/KokkosKineticDisPreConcAux

!syntax children /AuxKernels/KokkosKineticDisPreConcAux

!if-end!

!else
!include kokkos/kokkos_warning.md

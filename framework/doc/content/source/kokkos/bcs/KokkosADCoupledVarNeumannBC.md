# KokkosADCoupledVarNeumannBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ADCoupledVarNeumannBC](CoupledVarNeumannBC.md). See the original document for details.

## Example Input Syntax

!listing test/tests/kokkos/bcs/ad_coupled_var_neumann/kokkos_ad_coupled_var_neumann.i start=[right] end=[] include-end=true

!syntax parameters /BCs/KokkosADCoupledVarNeumannBC

!syntax inputs /BCs/KokkosADCoupledVarNeumannBC

!syntax children /BCs/KokkosADCoupledVarNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md

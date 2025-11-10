# KokkosCoupledVarNeumannBC

!if! function=hasCapability('kokkos')

This is the Kokkos version of [CoupledVarNeumannBC](CoupledVarNeumannBC.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/bcs/coupled_var_neumann/kokkos_coupled_var_neumann.i start=[right] end=[] include-end=true

!syntax parameters /BCs/KokkosCoupledVarNeumannBC

!syntax inputs /BCs/KokkosCoupledVarNeumannBC

!if-end!

!else
!include kokkos/kokkos_warning.md

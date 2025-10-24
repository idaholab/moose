# KokkosVariableTimeIntegrationAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [VariableTimeIntegrationAux](VariableTimeIntegrationAux.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/auxkernels/time_integration/kokkos_time_integration.i start=[SimpsonsTimeIntegrator] end=[] include-end=true

!syntax parameters /KokkosAuxKernels/KokkosVariableTimeIntegrationAux

!syntax inputs /KokkosAuxKernels/KokkosVariableTimeIntegrationAux

!if-end!

!else
!include kokkos/kokkos_warning.md

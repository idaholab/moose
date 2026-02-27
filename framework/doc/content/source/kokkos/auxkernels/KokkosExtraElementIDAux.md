# KokkosExtraElementIDAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [ExtraElementIDAux](ExtraElementIDAux.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/auxkernels/extra_element_id_aux/kokkos_extra_element_integer_aux.i start=[test_id] end=[] include-end=true

!syntax parameters /AuxKernels/KokkosExtraElementIDAux

!syntax inputs /AuxKernels/KokkosExtraElementIDAux

!syntax children /AuxKernels/KokkosExtraElementIDAux

!if-end!

!else
!include kokkos/kokkos_warning.md

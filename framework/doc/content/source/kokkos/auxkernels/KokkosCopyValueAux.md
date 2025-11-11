# KokkosCopyValueAux

!if! function=hasCapability('kokkos')

This is the Kokkos version of [CopyValueAux](CopyValueAux.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/auxkernels/copy_value_aux/kokkos_copy_old_aux.i start=[T_older] end=[] include-end=true

!syntax parameters /AuxKernels/KokkosCopyValueAux

!syntax inputs /AuxKernels/KokkosCopyValueAux

!syntax children /AuxKernels/KokkosCopyValueAux

!if-end!

!else
!include kokkos/kokkos_warning.md

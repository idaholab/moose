# KokkosPiecewiseConstant

!if! function=hasCapability('kokkos')

This is the Kokkos version of [PiecewiseConstant](PiecewiseConstant.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/functions/piecewise_constant/kokkos_piecewise_constant.i block=KokkosFunctions

!syntax parameters /KokkosFunctions/KokkosPiecewiseConstant

!syntax inputs /KokkosFunctions/KokkosPiecewiseConstant

!if-end!

!else
!include kokkos/kokkos_warning.md

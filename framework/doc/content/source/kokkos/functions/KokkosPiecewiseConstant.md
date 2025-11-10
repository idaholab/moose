# KokkosPiecewiseConstant

!if! function=hasCapability('kokkos')

This is the Kokkos version of [PiecewiseConstant](PiecewiseConstant.md). See the original document for details.

## Example Syntax

!listing test/tests/kokkos/functions/piecewise_constant/kokkos_piecewise_constant.i block=Functions

!syntax parameters /Functions/KokkosPiecewiseConstant

!syntax inputs /Functions/KokkosPiecewiseConstant

!if-end!

!else
!include kokkos/kokkos_warning.md

# KokkosEqualValueConstraint

!if! function=hasCapability('kokkos')

This is the Kokkos version of [EqualValueConstraint](EqualValueConstraint.md) without stabilization.
See the original document for details. The Lagrange multiplier may use a standard or a dual basis;
the dual basis coefficients of each secondary element are computed on the device from its mortar
segment quadrature.

## Example Input Syntax

!listing test/tests/kokkos/constraints/equal_value/equal_value.i block=Constraints

!syntax parameters /Constraints/KokkosEqualValueConstraint

!syntax inputs /Constraints/KokkosEqualValueConstraint

!syntax children /Constraints/KokkosEqualValueConstraint

!if-end!

!else
!include kokkos/kokkos_warning.md

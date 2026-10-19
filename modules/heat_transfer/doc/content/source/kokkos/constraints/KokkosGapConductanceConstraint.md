# KokkosGapConductanceConstraint

!if! function=hasCapability('kokkos')

This is the Kokkos version of [GapConductanceConstraint](GapConductanceConstraint.md) on the
reference configuration. See the original document for details. The gap width at each mortar
segment quadrature point is the distance from the secondary to the primary face along the
interpolated secondary normal; it is computed once on the host when the mortar segments are packed.
The Lagrange multiplier may use a standard or a dual basis.

!alert note
The Kokkos version always uses the correct edge dropping treatment: secondary elements partially
covered by the primary surface keep their Lagrange multiplier degrees of freedom and integrate over
their covered part.

## Example Input Syntax

!listing heat_transfer/test/tests/kokkos/gap_conductance/gap_conductance.i block=Constraints

!syntax parameters /Constraints/KokkosGapConductanceConstraint

!syntax inputs /Constraints/KokkosGapConductanceConstraint

!syntax children /Constraints/KokkosGapConductanceConstraint

!if-end!

!else
!include kokkos/kokkos_warning.md

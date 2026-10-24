# KokkosGapConductanceConstraint

!if! function=hasCapability('kokkos')

This is the Kokkos version of [GapConductanceConstraint](GapConductanceConstraint.md). See the
original document for details. The gap width at each mortar segment quadrature point is the distance
from the secondary to the primary face along the interpolated secondary normal. The Lagrange
multiplier may use a standard or a dual basis.

With [!param](/Constraints/KokkosGapConductanceConstraint/use_displaced_mesh) set to true, the
constraint is posed on the current configuration given by the displacements of the `[Mesh]` block,
which it couples without a `displacements` parameter. The mortar segments come from the displaced
mesh and are rebuilt before every evaluation, and the Jacobian includes the derivatives of the gap
with respect to the displacements, holding the normals and the segment quadrature fixed, as the
original version does.

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

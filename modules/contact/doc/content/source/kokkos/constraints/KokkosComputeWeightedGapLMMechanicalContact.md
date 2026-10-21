# KokkosComputeWeightedGapLMMechanicalContact

!if! function=hasCapability('kokkos')

!syntax description /Constraints/KokkosComputeWeightedGapLMMechanicalContact

This is the Kokkos counterpart of
[ComputeWeightedGapLMMechanicalContact](ComputeWeightedGapLMMechanicalContact.md). At each secondary
node $j$ it enforces the frictionless contact conditions with the nonlinear complementarity function

!equation
R_j = \min(\lambda_j, c_j \tilde{g}_j),

where $\tilde{g}_j$ is the weighted gap of
[KokkosWeightedGapUserObject](KokkosWeightedGapUserObject.md) and $c_j$ is
[!param](/Constraints/KokkosComputeWeightedGapLMMechanicalContact/c), divided by the normalization of
the weighted gap when [!param](/Constraints/KokkosComputeWeightedGapLMMechanicalContact/normalize_c)
is true. The Jacobian includes the derivatives of the nodal normals with respect to the positions of
the secondary face nodes, so it is exact for a fixed mortar segmentation.

The constraint is posed on the current configuration given by the displacements of the `[Mesh]`
block, which it couples without displacement parameters. It is used together with
[KokkosNormalMortarMechanicalContact](KokkosNormalMortarMechanicalContact.md), which applies the
contact force. The Lagrange multiplier must be a first-order Lagrange variable, and the correct edge
dropping treatment is always used.

## Example Input Syntax

!listing contact/test/tests/kokkos/mortar/frictionless_weighted_gap.i block=Constraints

!syntax parameters /Constraints/KokkosComputeWeightedGapLMMechanicalContact

!syntax inputs /Constraints/KokkosComputeWeightedGapLMMechanicalContact

!syntax children /Constraints/KokkosComputeWeightedGapLMMechanicalContact

!if-end!

!else
!include kokkos/kokkos_warning.md

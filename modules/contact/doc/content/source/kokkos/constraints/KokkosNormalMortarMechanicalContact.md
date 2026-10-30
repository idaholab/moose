# KokkosNormalMortarMechanicalContact

!if! function=hasCapability('kokkos')

!syntax description /Constraints/KokkosNormalMortarMechanicalContact

This is the Kokkos counterpart of
[NormalMortarMechanicalContact](NormalMortarMechanicalContact.md). One object applies the contact
traction

!equation
\boldsymbol{t} = \sum_j \psi_j \lambda_j \boldsymbol{n}_j

to every displacement component of the `[Mesh]` displacements, with a positive sign on the secondary
face and a negative sign on the primary face. Here $\psi_j$ is the basis function of the Lagrange
multiplier at secondary node $j$ and $\boldsymbol{n}_j$ is the nodal normal. The Jacobian includes
the derivatives with respect to the multiplier and, through the nodal normals, to the positions of
the secondary face nodes.

The constraint leaves the rows of the Lagrange multiplier to
[KokkosComputeWeightedGapLMMechanicalContact](KokkosComputeWeightedGapLMMechanicalContact.md). The
displacements must be solution variables of the system of the Lagrange multiplier.

## Example Input Syntax

!listing contact/test/tests/kokkos/mortar/frictionless_weighted_gap.i block=Constraints

!syntax parameters /Constraints/KokkosNormalMortarMechanicalContact

!syntax inputs /Constraints/KokkosNormalMortarMechanicalContact

!syntax children /Constraints/KokkosNormalMortarMechanicalContact

!if-end!

!else
!include kokkos/kokkos_warning.md

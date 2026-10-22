# KokkosCartesianMortarMechanicalContact

!if! function=hasCapability('kokkos')

!syntax description /Constraints/KokkosCartesianMortarMechanicalContact

This is the Kokkos counterpart of [CartesianMortarMechanicalContact](CartesianMortarMechanicalContact.md).
One object applies the traction $\boldsymbol{t} = \sum_j \psi_j \boldsymbol{\lambda}_j$ of a
Cartesian Lagrange multiplier to every displacement component of the `[Mesh]` displacements, with a
positive sign on the secondary face and a negative sign on the primary face. It leaves the rows of
the multiplier to a constraint such as
[KokkosComputeCartesianLMMechanicalContact](KokkosComputeCartesianLMMechanicalContact.md).

## Example Input Syntax

!listing contact/test/tests/kokkos/mortar/frictional_cartesian.i block=Constraints

!syntax parameters /Constraints/KokkosCartesianMortarMechanicalContact

!syntax inputs /Constraints/KokkosCartesianMortarMechanicalContact

!syntax children /Constraints/KokkosCartesianMortarMechanicalContact

!if-end!

!else
!include kokkos/kokkos_warning.md

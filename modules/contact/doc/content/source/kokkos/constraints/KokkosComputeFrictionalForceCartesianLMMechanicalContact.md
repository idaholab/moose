# KokkosComputeFrictionalForceCartesianLMMechanicalContact

!if! function=hasCapability('kokkos')

!syntax description /Constraints/KokkosComputeFrictionalForceCartesianLMMechanicalContact

This is the Kokkos counterpart of
[ComputeFrictionalForceCartesianLMMechanicalContact](ComputeFrictionalForceCartesianLMMechanicalContact.md).
The Lagrange multiplier is a Cartesian vector $\boldsymbol{\lambda}_j$ at each secondary node $j$,
one first-order Lagrange variable per direction. With the nodal normal $\boldsymbol{n}_j$, it is
split into the normal pressure $\lambda_n = \boldsymbol{\lambda}_j \cdot \boldsymbol{n}_j$ and the
tangential traction $\boldsymbol{\lambda}_t = (\mathbf{I} - \boldsymbol{n}_j \boldsymbol{n}_j^T)
\boldsymbol{\lambda}_j$, so no tangent frame is needed. The normal contact conditions are enforced by

!equation
R_n = \min(\lambda_n, c \tilde{g}_j),

and Coulomb friction by the Hueber-Stadler-Wohlmuth residual

!equation
\boldsymbol{R}_t = \max(r, |\boldsymbol{q}_t|) \boldsymbol{\lambda}_t - r \boldsymbol{q}_t, \quad
\boldsymbol{q}_t = \boldsymbol{\lambda}_t + c_t \tilde{\boldsymbol{s}}_t, \quad
r = \mu \max(0, \lambda_n - c \tilde{g}_j),

where $\tilde{g}_j$ is the weighted gap and $\tilde{\boldsymbol{s}}_t$ the tangential part of the
weighted relative displacement increment over the time step, both from
[KokkosWeightedGapUserObject](KokkosWeightedGapUserObject.md) with
[!param](/UserObjects/KokkosWeightedGapUserObject/compute_slip) set to true. Below a normal pressure
of [!param](/Constraints/KokkosComputeFrictionalForceCartesianLMMechanicalContact/epsilon) the
tangential traction is set to zero.

The nodal residual is differentiated with forward-mode dual numbers. Its Jacobian includes the
derivatives with respect to the multiplier, the nodal normal, the weighted gap vector, and the
weighted slip, for a fixed mortar segmentation. It is used together with
[KokkosCartesianMortarMechanicalContact](KokkosCartesianMortarMechanicalContact.md), which applies
the traction.

## Example Input Syntax

!listing contact/test/tests/kokkos/mortar/frictional_cartesian.i block=Constraints

!syntax parameters /Constraints/KokkosComputeFrictionalForceCartesianLMMechanicalContact

!syntax inputs /Constraints/KokkosComputeFrictionalForceCartesianLMMechanicalContact

!syntax children /Constraints/KokkosComputeFrictionalForceCartesianLMMechanicalContact

!if-end!

!else
!include kokkos/kokkos_warning.md

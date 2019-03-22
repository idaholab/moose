# ADGravity

!syntax description /Kernels/ADGravity<RESIDUAL>

## Description

The kernel `ADGravity` provides a body force term in the stress divergence equilibrium
equation to account for ADGravity force due to self weight.
In a continuum mechanics problem, momentum conservation is prescribed assuming
static equilibrium at each time increment,
\begin{equation}
\nabla \cdot \boldsymbol{\sigma} + g = 0,
\end{equation}
where $\boldsymbol{\sigma}$ is the Cauchy stress tensor and $\boldsymbol{g}$ is
the ADGravity body force per unit mass.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/ADGravity/ad_gravity_test.i block=Kernels/ADGravity_y

!syntax parameters /Kernels/ADGravity<RESIDUAL>

!syntax inputs /Kernels/ADGravity<RESIDUAL>

!syntax children /Kernels/ADGravity<RESIDUAL>

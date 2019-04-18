# ADGravity

!syntax description /Kernels/ADGravity

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

!listing modules/tensor_mechanics/test/tests/gravity/ad_gravity_test.i block=Kernels/gravity_y

!syntax parameters /Kernels/ADGravity

!syntax inputs /Kernels/ADGravity

!syntax children /Kernels/ADGravity

# Gravity
!syntax description /Kernels/Gravity

## Description
The kernel `Gravity` provides a body force term in the stress divergence equilibrium equation to account for gravity force due to self weight.
In a continuum mechanics problem, momentum conservation is prescribed assuming static equilibrium at each time increment,
\begin{equation}
\nabla \cdot \mathbf{\sigma} + g = 0,
\end{equation}
where $\mathbf{\sigma}$ is the Cauchy stress tensor and $\mathbf{g}$ is the gravity body force per unit mass.

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/gravity/gravity_test.i block=Kernels/gravity_y

!syntax parameters /Kernels/Gravity

!syntax inputs /Kernels/Gravity

!syntax children /Kernels/Gravity

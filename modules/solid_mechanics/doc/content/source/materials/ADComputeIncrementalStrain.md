# ADComputeIncrementalStrain

!syntax description /Materials/ADComputeIncrementalStrain

## Description

The material `ADComputeIncrementalStrain` is designed for linear elasticity
problems formulated within an incremental framework.  As with
[ADComputeSmallStrain](/ADComputeSmallStrain.md), this material is useful for
verifying material models with hand calculations because of the simplified
strain calculations.  As in the small strain material, the incremental small
strain class assumes the gradient of displacement with respect to position is
much smaller than unity, and the squared displacement gradient term is neglected
in the small strain definition to give:

\begin{equation}
\epsilon = \frac{1}{2} \left( u \nabla + \nabla u \right) \quad when \quad \frac{\partial u}{ \partial x} << 1
\end{equation}

As the class name suggests, `ADComputeIncrementalStrain` is an incremental
formulation.  The stress increment is calculated from the current strain
increment at each time step.  In this class, the rotation tensor is defined to
be the rank-2 Identity tensor: no rotations are allowed in the model. Stateful
properties, including `strain_old` and `stress_old`, are stored. This
incremental small strain material is useful as a component of verifying more
complex finite incremental strain-stress calculations.

!syntax parameters /Materials/ADComputeIncrementalStrain

!syntax inputs /Materials/ADComputeIncrementalStrain

!syntax children /Materials/ADComputeIncrementalStrain

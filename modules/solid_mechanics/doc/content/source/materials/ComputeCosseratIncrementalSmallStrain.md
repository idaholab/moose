# ComputeCosseratIncrementalSmallStrain

!syntax description /Materials/ComputeCosseratIncrementalSmallStrain

Incremental small strain tensors rely on stateful material properties to keep track of the 'old' (previous time step)
value of the material properties.
The Cosserat small strain tensor and its increment are computed from the gradients of the displacements and
the Cosserat rotation variables in the current and previous time steps as:

!equation
\gamma_{increment} = \nabla_{j}u_{i} - \nabla_{j}u_{i,old} + \epsilon_{ijkl}\theta_{c}^{k} - \epsilon_{ijkl}\theta_{c,old}^{k}

!equation
\gamma_{ij} = \gamma_{old, ij} + \gamma_{inc}

where

- the $_{inc}$ index indicates an increment, which is also defined as a material property
- the $_{old}$ index indicates a material property computed at the previous time step
- $\gamma_{ij}$ are the (i,j) components of the strain tensor
- $\nabla_{j}u_{i}$ are the  (i,j) components of the gradients of the displacements
- $\epsilon_{ijkl}$ is the permutation tensor, where the $k$ and $l$ indexes indicate sums
- $\theta_{c}^{k}$ are the Cosserat rotation variables

The curvature tensor $C$ and its increment are also computed by this `Material` from the gradients of the Cosserat rotations as:

!equation
C_{inc} = \nabla_{j}\theta_{c}^{i} - \nabla_{j}\theta_{c,old}^{i}

!equation
C = C_{old} + C_{inc}

!alert note
This object is part of the Cosserat mechanics model. See the theory manual (at [solid_mechanics/doc/theory/cosserat.pdf](https://github.com/idaholab/moose/tree/next/modules/solid_mechanics/doc/theory/cosserat.pdf))
for more explanation.

!syntax parameters /Materials/ComputeCosseratIncrementalSmallStrain

!syntax inputs /Materials/ComputeCosseratIncrementalSmallStrain

!syntax children /Materials/ComputeCosseratIncrementalSmallStrain

# ComputeCosseratSmallStrain

!syntax description /Materials/ComputeCosseratSmallStrain

The Cosserat small strain tensor is computed from the gradients of the displacements and
the Cosserat rotation variables as:

!equation
\gamma_{ij} = \nabla_{j}u_{i} + \epsilon_{ijkl}\theta_{c}^{k}

where

- $\gamma_{ij}$ are the (i,j) components of the strain tensor
- $\nabla_{j}u_{i}$ are the  (i,j) components of the gradients of the displacements
- $\epsilon_{ijkl}$ is the permutation tensor, where the $k$ and $l$ indexes indicate sums
- $\theta_{c}^{k}$ are the Cosserat rotation variables

The curvature tensor $C$ is also computed by this `Material` from the gradients of the Cosserat rotations as:

!equation
C = \nabla_{j}\theta_{c}^{i}

!alert note
This object is part of the Cosserat mechanics model. See the theory manual (at [solid_mechanics/doc/theory/cosserat.pdf](https://github.com/idaholab/moose/tree/next/modules/solid_mechanics/doc/theory/cosserat.pdf))
for more explanation.

!syntax parameters /Materials/ComputeCosseratSmallStrain

!syntax inputs /Materials/ComputeCosseratSmallStrain

!syntax children /Materials/ComputeCosseratSmallStrain

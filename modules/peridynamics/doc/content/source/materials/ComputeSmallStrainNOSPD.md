# Correspondence Small Strain Material

## Description

The `ComputeSmallStrainNOSPD` Material calculates the strain tensors based on [deformation gradient](peridynamics/DeformationGradients.md) for the peridynamic correspondence models under small strain assumptions.

Given the deformation gradient $\mathbf{F}$, the total strain tensor can be obtained as

\begin{equation}
  \boldsymbol{\epsilon}_{\text{total}} = \frac{1}{2} \left( \mathbf{F}^{T} + \mathbf{F} \right) - \mathbf{I}
\end{equation}

and the elastic strain tensor can be obtained by subtracting the eigen strain tensor from the total strain tensor as

\begin{equation}
  \boldsymbol{\epsilon}_{\text{elastic}} = \boldsymbol{\epsilon}_{\text{total}} - \boldsymbol{\epsilon}_{\text{eigen}}
\end{equation}

The computed elastic strain tensor is then used in stress calculator to calculate the stress tensor.

Note that Material `ComputeSmallStrainNOSPD` can be used in general 3D and plane strain modeling and simulation.

!syntax parameters /Materials/ComputeSmallStrainNOSPD

!syntax inputs /Materials/ComputeSmallStrainNOSPD

!syntax children /Materials/ComputeSmallStrainNOSPD

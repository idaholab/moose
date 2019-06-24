# Correspondence Small Strain Material

## Description

Material `ComputeSmallStrainNOSPD` calculates the [bond-associated deformation gradient](peridynamics/DeformationGradients.md) used in self-stabilized non-ordinary state-based peridynamic mechanics model under small strain assumptions.

Given the bond-associated deformation gradient $\mathbf{F}_{\mathbf{\xi}}$, the bond-associated total strain tensor can be obtained as

\begin{equation}
  \boldsymbol{\epsilon}_{\mathbf{\xi}, \text{total}} = \frac{1}{2} \left( \mathbf{F}_{\mathbf{\xi}}^{T} \mathbf{F}_{\mathbf{\xi}} - \mathbf{I} \right)
\end{equation}

and the elastic strain tensor can be obtained by subtracting the eigen strain tensor from the total strain tensor as

\begin{equation}
  \boldsymbol{\epsilon}_{\mathbf{\xi}, \text{elastic}} = \boldsymbol{\epsilon}_{\mathbf{\xi}, \text{total}} - \boldsymbol{\epsilon}_{\mathbf{\xi}, \text{eigen}}
\end{equation}

The computed bond-associated elastic strain tensor is then used in stress calculator to calculate bond-associated stress tensor.

Note that Material `ComputeSmallStrainNOSPD` can be used in general 3D and plane strain modeling and simulation.

!syntax parameters /Materials/ComputeSmallStrainNOSPD

!syntax inputs /Materials/ComputeSmallStrainNOSPD

!syntax children /Materials/ComputeSmallStrainNOSPD

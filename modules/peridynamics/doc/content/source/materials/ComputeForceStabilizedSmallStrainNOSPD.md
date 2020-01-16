# Force-stabilized Non-ordinary State-based Peridynamic Correspondence Material

## Description

Material `ComputeForceStabilizedSmallStrainNOSPD` calculates the [deformation gradient](peridynamics/DeformationGradients.md) and its derivatives used in fictitious bond force stabilized non-ordinary state-based peridynamic mechanics model under infinitesimal strain assumptions.

Given the deformation gradient $\mathbf{F}$, the total strain tensor can be obtained as

\begin{equation}
  \boldsymbol{\epsilon}_{\text{total}} = \frac{1}{2} \left( \mathbf{F}^{T} \mathbf{F} - \mathbf{I} \right)
\end{equation}

and the elastic strain tensor can be obtained by subtracting the eigen strain tensor from the total strain tensor as

\begin{equation}
  \boldsymbol{\epsilon}_{\text{elastic}} = \boldsymbol{\epsilon}_{\text{total}} - \boldsymbol{\epsilon}_{\text{eigen}}
\end{equation}

The computed elastic strain tensor is then used in stress calculator class to compute stress tensor.

!syntax parameters /Materials/ComputeForceStabilizedSmallStrainNOSPD

!syntax inputs /Materials/ComputeForceStabilizedSmallStrainNOSPD

!syntax children /Materials/ComputeForceStabilizedSmallStrainNOSPD

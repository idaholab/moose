# Force-stabilized Non-ordinary State-based Peridynamic Mechanics Kernel

## Description

The `ForceStabilizedSmallStrainMechanicsNOSPD` Kernel calculates the residual and jacobian of the force density integral for fictitious bond force stabilized non-ordinary state-based model under infinitesimal strain assumptions.

## Force state

With the fictitious bond force to stabilized the formulation, the force state for each `Edge2` element, i.e., bond, is

\begin{equation}
  \mathbf{f} \left( \mathbf{X}, \mathbf{X}^{\prime}, t \right) = \left( \underline{\mathbf{T}} \left[ \mathbf{X}, t \right] \left\langle \mathbf{X}^{\prime} - \mathbf{X} \right\rangle - \underline{\mathbf{T}} \left[ \mathbf{X}^{\prime}, t \right] \left\langle \mathbf{X} - \mathbf{X}^{\prime} \right\rangle \right) + \mathbf{f}_{s} \left( \mathbf{X}^{\prime} - \mathbf{X}, t \right)
\end{equation}
with
\begin{equation}
  \underline{\mathbf{T}} \left[ \mathbf{X}, t \right] \left\langle \mathbf{X}^{\prime} - \mathbf{X} \right\rangle = \underline{\omega} \left\langle \boldsymbol{\xi} \right\rangle \mathbf{P}_{\mathbf{X}} \mathbf{K}_{\mathbf{X}}^{-1} \boldsymbol{\xi}
\end{equation}

\begin{equation}
  \underline{\mathbf{T}} \left[ \mathbf{X}^{\prime}, t \right] \left\langle \mathbf{X} - \mathbf{X}^{\prime} \right\rangle = - \underline{\omega} \left\langle \boldsymbol{\xi} \right\rangle \mathbf{P}_{\mathbf{X}^{\prime}} \mathbf{K}_{\mathbf{X}^{\prime}}^{-1} \boldsymbol{\xi}
\end{equation}

\begin{equation}
  \mathbf{f}_{s} \left( \mathbf{X}^{\prime} - \mathbf{X}, t \right) = E \Delta x \underline{\omega} \left\langle \boldsymbol{\xi} \right\rangle \left( \mathbf{u}_{\mathbf{X}^{\prime}} - \mathbf{u}_{\mathbf{X}} \right)
\end{equation}
where $E$ is young's modulus, $\Delta x$ is grid spacing for regular gird structure or average grid spacing for irregular grid structure, and $\mathbf{u}$ is the solution vector.

We don't distinguish Cauchy stress with the first Piola-Kichhoff stress under infinitesimal strain assumptions. The stress retrieved from `ComputeLinearElasticStress` is directly used in the force state calculation.

!syntax parameters /Kernels/ForceStabilizedSmallStrainMechanicsNOSPD

!syntax inputs /Kernels/ForceStabilizedSmallStrainMechanicsNOSPD

!syntax children /Kernels/ForceStabilizedSmallStrainMechanicsNOSPD

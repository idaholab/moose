# Self-stabilized Non-ordinary State-based Peridynamic Finite Strain Mechanics Kernel

## Description

Kernel `FiniteStrainMechanicsNOSPD` calculates the residual and jacobian of the force density integral for self-stabilized non-ordinary state-based models under finite strain assumptions.

## Force state

The force state for each `Edge2` element, i.e., bond, is

\begin{equation}
  \mathbf{f}\left(\mathbf{X},\mathbf{X}^{\prime},t\right) = \underline{\mathbf{T}}\left[\mathbf{X},t\right]\left\langle \mathbf{X}^{\prime}-\mathbf{X} \right\rangle - \underline{\mathbf{T}}\left[\mathbf{X}^{\prime},t\right]\left\langle \mathbf{X}-\mathbf{X}^{\prime} \right\rangle
\end{equation}
with
\begin{equation}
  \underline{\mathbf{T}}\left[\mathbf{X},t\right]\left\langle \mathbf{X}^{\prime}-\mathbf{X} \right\rangle =
   \frac{\int_{\mathcal{H}_{\mathbf{X}} \cap h_{\mathbf{X},\boldsymbol{\xi}}}1dV_{\mathbf{X}^{\prime}}}{\int_{\mathcal{H}_{\mathbf{X}}}1 dV_{\mathbf{X}^{\prime}}} \underline{\omega}\left\langle \boldsymbol{\xi} \right\rangle \mathbf{P}_{\mathbf{X,\boldsymbol{\xi}}} \mathbf{K}_{\mathbf{X,\boldsymbol{\xi}}}^{-1} \boldsymbol{\xi}
\end{equation}

\begin{equation}
  \underline{\mathbf{T}}\left[\mathbf{X}^{\prime},t\right]\left\langle \mathbf{X}-\mathbf{X}^{\prime} \right\rangle = - \frac{\int_{\mathcal{H}_{\mathbf{X}^{\prime}} \cap h_{\mathbf{X}^{\prime},-\boldsymbol{\xi}}}1dV_{\mathbf{X}}}{\int_{\mathcal{H}_{\mathbf{X}^{\prime}}}1 dV_{\mathbf{X}}} \underline{\omega}\left\langle \boldsymbol{\xi} \right\rangle \mathbf{P}_{\mathbf{X}^{\prime},-\boldsymbol{\xi}} \mathbf{K}_{\mathbf{X}^{\prime},-\boldsymbol{\xi}}^{-1} \boldsymbol{\xi}
\end{equation}

The stress tensor retrieved from `Compute**Stress` is Cauchy stress and conversion to the first Piola-Kichhoff stress is required for finite strain case.

\begin{equation}
  \mathbf{P} = J \boldsymbol{\sigma} \mathbf{F}^{-T}
\end{equation}
where $J$ is the determinant of deformation gradient $\mathbf{F}$.

!syntax parameters /Kernels/FiniteStrainMechanicsNOSPD

!syntax inputs /Kernels/FiniteStrainMechanicsNOSPD

!syntax children /Kernels/FiniteStrainMechanicsNOSPD

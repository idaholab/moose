# Horizon-stabilized Non-ordinary State-based Peridynamic Small Strain Mechanics Kernel

## Description

The `HorizonStabilizedSmallStrainMechanicsNOSPD` Kernel calculates the residual and Jacobian entries of the force density integral for horizon-stabilized non-ordinary state-based peridynamic models under infinitesimal strain assumptions.

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

We don't distinguish Cauchy stress with the first Piola-Kichhoff stress under infinitesimal strain assumptions. The stress retrieved from `ComputeLinearElasticStress` is directly used in the force state calculation.

\begin{equation}
  \mathbf{P} = \boldsymbol{\sigma}
\end{equation}

!syntax parameters /Kernels/HorizonStabilizedSmallStrainMechanicsNOSPD

!syntax inputs /Kernels/HorizonStabilizedSmallStrainMechanicsNOSPD

!syntax children /Kernels/HorizonStabilizedSmallStrainMechanicsNOSPD

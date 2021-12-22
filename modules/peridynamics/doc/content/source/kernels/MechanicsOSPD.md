# Ordinary State-based Peridynamic Mechanics Kernel

## Description

The `MechanicsOSPD` Kernel calculates the residual and jacobian of the force density integral for 2D, and 3D ordinary state-based peridynamic models under infinitesimal strain assumptions.

## Force state

The force state for each `Edge2` element, i.e., bond, is

\begin{equation}
  \mathbf{f} \left( \mathbf{X}, \mathbf{X}^{\prime}, t \right) = \left( \underline{\mathbf{T}} \left( \mathbf{X}, t\right) - \underline{\mathbf{T}} \left( \mathbf{X}^{\prime},t\right) \right) \cdot\mathbf{M}
\end{equation}
where $\mathbf{M}$ is the unit vector in deformed configuration.

\begin{equation}
  \underline{\mathbf{T}}\left(\mathbf{X}, t\right) = 2 \underline{\omega} \left\langle \boldsymbol{\xi} \right\rangle \left( d_{\mathbf{X}} \cdot a \cdot \theta_{\mathbf{X}} + b \cdot \boldsymbol{\xi} \cdot s\right)
\end{equation}

\begin{equation}
  \underline{\mathbf{T}} \left( \mathbf{X}, t \right) = -2 \underline{\omega} \left\langle \boldsymbol{\xi} \right\rangle \left( d_{\mathbf{X}^{\prime}} \cdot a \cdot \theta_{\mathbf{X}^{\prime}} + b \cdot \boldsymbol{\xi} \cdot s \right)
\end{equation}
where $s$ is the bond stretch, $\theta$ is the dilatation at a material point, parameters $a$, $b$ and $d$ are model parameters which can be expressed in terms of the material constants.

More details on the residual and Jacobian formulation can be found in Ref. [!citep](Hu2018irregular).

!syntax parameters /Kernels/MechanicsOSPD

!syntax inputs /Kernels/MechanicsOSPD

!syntax children /Kernels/MechanicsOSPD

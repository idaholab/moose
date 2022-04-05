# TotalLagrangianRZStressDivergence

!syntax description /Kernels/TotalLagrangianRZStressDivergence

## Description

The `TotalLagrangianRZStressDivergence` derives from `TotalLagrangianStressDivergence`
and redefines the gradient operator in axisymmetric RZ coordinates.

Recall that the residual and the Jacobian of the `TotalLagrangianStressDivergence` are given as
\begin{equation}
  \begin{aligned}
    R^{\alpha} &= \int_{V}P_{iK}\phi_{i,K}^{\alpha}dV \\
    J^{\alpha\beta} &= \int_{V}\phi_{i,J}^{\alpha}T_{iJkL}^{\prime}\psi_{k,L}^{\beta}dV
  \end{aligned}
\end{equation}

In cylindrical coordinates, the gradient operator (with respect to the reference coordinates) is given as
\begin{equation}
  (\cdot)_{,\textbf{X}} = (\cdot)_{,R}\textbf{e}_R + (\cdot)_{,Z}\textbf{e}_Z + \frac{1}{R}(\cdot)_{,\Theta}\textbf{e}_\Theta,
\end{equation}
and the deformation gradient is given as
\begin{equation}
  \begin{aligned}
    \boldsymbol{F} &= (1+u_{r,R}) \textbf{e}_r\textbf{e}_R + u_{r,Z} \textbf{e}_r\textbf{e}_Z + \left(1+\frac{u_r}{R}\right)\textbf{e}_\theta\textbf{e}_\Theta + u_{z,R} \textbf{e}_z\textbf{e}_R + (1+u_{z,Z}) \textbf{e}_z\textbf{e}_Z
  \end{aligned}
\end{equation}

Hence, the gradient of the test functions are
\begin{equation}
  \begin{aligned}
    \boldsymbol{\phi}^r &= \phi_{r,R}\textbf{e}_r\textbf{e}_R + \phi_{r,Z}\textbf{e}_r\textbf{e}_Z + \frac{\phi_{r}}{R}\textbf{e}_\theta\textbf{e}_\Theta, \\
    \boldsymbol{\phi}^z &= \phi_{z,R}\textbf{e}_z\textbf{e}_R + \phi_{z,Z}\textbf{e}_z\textbf{e}_Z
  \end{aligned}
\end{equation}
and similarly the trial strain is
\begin{equation}
  \begin{aligned}
    \boldsymbol{\psi}^r &= \psi_{r,R}\textbf{e}_r\textbf{e}_R + \psi_{r,Z}\textbf{e}_r\textbf{e}_Z + \frac{\psi_{r}}{R}\textbf{e}_\theta\textbf{e}_\Theta, \\
    \boldsymbol{\psi}^z &= \psi_{z,R}\textbf{e}_z\textbf{e}_R + \psi_{z,Z}\textbf{e}_z\textbf{e}_Z
  \end{aligned}
\end{equation}

!syntax parameters /Kernels/TotalLagrangianRZStressDivergence

!syntax inputs /Kernels/TotalLagrangianRZStressDivergence

!syntax children /Kernels/TotalLagrangianRZStressDivergence

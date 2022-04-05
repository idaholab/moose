# ComputeLagrangianAxisymmetricRZStrain

!syntax description /Materials/ComputeLagrangianAxisymmetricRZStrain

## Overview

The `ComputeLagrangianAxisymmetricRZStrain` class calculates strain in RZ coordinates. A point $\textbf{X}$ in the reference frame can be expressed in terms of the radius $R$ and the axial position $Z$:
\begin{equation}
  \textbf{X} = R \textbf{e}_R + Z \textbf{e}_Z,
\end{equation}
where $\textbf{e}_R$ and $\textbf{e}_Z$ are unit vectors in the reference frame. At a deformed state, the material point can be expressed in terms of the current radius $r$ and the current axial position $z$ as
\begin{equation}
  \textbf{x} = r \textbf{e}_r + Z \textbf{e}_z,
\end{equation}
where $\textbf{e}_r$ and $\textbf{e}_z$ are unit vectors in the current frame. The general motion at a point $\textbf{X}$ in cylindrical coordinates can be written as
\begin{equation}
  \begin{aligned}
    r(R,Z,\Theta) = R + u_r(R,Z,\Theta), \\
    z(R,Z,\Theta) = Z + u_z(R,Z,\Theta), \\
    \theta(R,Z,\Theta) = \Theta + u_\theta(R,Z,\Theta).
  \end{aligned}
\end{equation}
Under axisymmetry assumptions, i.e. the motion is independent of $\Theta$ (and only rigid-body rotation is permitted), the axisymmetric motion can be expressed as
\begin{equation}
  \begin{aligned}
    r(R,Z) = R + u_r(R,Z), \\
    z(R,Z) = Z + u_z(R,Z), \\
    \theta = \Theta + u_\theta.
  \end{aligned}
\end{equation}
Hence, $u_r$ and $u_z$ are the two degrees of freedom we are solving for.

In cylindrical coordinates, the gradient operator (with respect to the reference coordinates) is written as
\begin{equation}
  (\cdot)_{,\textbf{X}} = (\cdot)_{,R}\textbf{e}_R + (\cdot)_{,Z}\textbf{e}_Z + \frac{1}{R}(\cdot)_{,\Theta}\textbf{e}_\Theta,
\end{equation}
where comma denotes partial differentiation. The deformation gradient can therefore be written as
\begin{equation}
  \begin{aligned}
    \boldsymbol{F} &= \textbf{x}_{,\textbf{X}} \\
    &= r_{,R} \textbf{e}_r\textbf{e}_R + r_{,Z} \textbf{e}_r\textbf{e}_Z + \frac{r}{R}\textbf{e}_\theta\textbf{e}_\Theta + z_{,R} \textbf{e}_z\textbf{e}_R + z_{,Z} \textbf{e}_z\textbf{e}_Z \\
    &= (1+u_{r,R}) \textbf{e}_r\textbf{e}_R + u_{r,Z} \textbf{e}_r\textbf{e}_Z + \left(1+\frac{u_r}{R}\right)\textbf{e}_\theta\textbf{e}_\Theta + u_{z,R} \textbf{e}_z\textbf{e}_R + (1+u_{z,Z}) \textbf{e}_z\textbf{e}_Z \\
    &= \boldsymbol{1} +
    \begin{bmatrix}
      u_{r,R} & u_{r,Z} & 0 \\
      u_{z,R} & u_{z,Z} & 0 \\
      0 & 0 & \frac{u_r}{R}
    \end{bmatrix}
  \end{aligned}
\end{equation}

!syntax parameters /Materials/ComputeLagrangianAxisymmetricRZStrain

!syntax inputs /Materials/ComputeLagrangianAxisymmetricRZStrain

!syntax children /Materials/ComputeLagrangianAxisymmetricRZStrain

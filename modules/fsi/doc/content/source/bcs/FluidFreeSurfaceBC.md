# Free surface fluid boundary condition

!syntax description /BCs/FluidFreeSurfaceBC

## Description

The free surface fluid BC applies a mixed Dirichlet-Neumann condition to simulate
gravity waves. This condition is as follows:

\begin{equation}
    \label{eqn:Free1}
    \frac{\partial p}{\partial z} = -\alpha~\frac{\partial^2p}{\partial t^2}
\end{equation}

where, $p$ is the fluid pressure and $g$ is the acceleration due to gravity. This BC is part of the fluid-structure interaction codes. Please refer to [fluid-structure interaction using acoustics](/fsi_acoustics.md) for the theoretical details.

!syntax parameters /BCs/FluidFreeSurfaceBC

!syntax inputs /BCs/FluidFreeSurfaceBC

!syntax children /BCs/FluidFreeSurfaceBC

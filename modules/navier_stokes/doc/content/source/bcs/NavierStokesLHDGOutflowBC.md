# NavierStokesLHDGOutflowBC

This class implements an outflow boundary condition for use with the
hybridized kernel [NavierStokesLHDGKernel.md]. The condition imposed is

\begin{equation}
\vec{n}p -\mu\nabla\vec{u}\cdot{n} = \vec{0}
\end{equation}

where $\vec{n}$ is the outward-facing normal on the boundary, $p$ is the
pressure, $\nu$ is the kinematic viscosity, and $\vec{u}$ is the velocity.

!syntax parameters /BCs/NavierStokesLHDGOutflowBC

!syntax inputs /BCs/NavierStokesLHDGOutflowBC

!syntax children /BCs/NavierStokesLHDGOutflowBC

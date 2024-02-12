# NavierStokesHybridizedOutflowBC

This class implements an outflow boundary condition for use with the
hybridized kernel [NavierStokesHybridizedKernel.md]. The condition imposed is

\begin{equation}
\vec{n}p -\nu\nabla\vec{u}\cdot{n} = \vec{0}
\end{equation}

where $\vec{n}$ is the outward-facing normal on the boundary, $p$ is the
pressure, $\nu$ is the kinematic viscosity, and $\vec{u}$ is the velocity.

!syntax parameters /HybridizedBCs/NavierStokesHybridizedOutflowBC

!syntax inputs /HybridizedBCs/NavierStokesHybridizedOutflowBC

!syntax children /HybridizedBCs/NavierStokesHybridizedOutflowBC

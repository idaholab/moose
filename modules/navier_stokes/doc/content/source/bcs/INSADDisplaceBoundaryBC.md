# INSADDisplaceBoundaryBC

!syntax description /BCs/INSADDisplaceBoundaryBC

This boundary condition displaces a boundary in proportion to a coupled
velocity. It is a strongly enforced boundary condition on every displacement
node with a residual of the form

\begin{equation}
u - (u_{old} + \Delta t \vec{v}_i)
\end{equation}

where $u$ denotes a given displacement vector component, $u_{old}$ is the
previous timestep's value of the displacement, $\Delta t$ is the timestep, and
$\vec{v}_i$ is a component of the coupled velocity vector, where $i$ denotes the
component.

!syntax parameters /BCs/INSADDisplaceBoundaryBC

!syntax inputs /BCs/INSADDisplaceBoundaryBC

!syntax children /BCs/INSADDisplaceBoundaryBC

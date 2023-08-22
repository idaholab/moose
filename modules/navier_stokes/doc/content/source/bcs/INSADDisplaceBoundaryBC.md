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

Note that the formula above is appropriate for an [ImplicitEuler.md]
discretization of time derivatives. Consequently this class will error if the
time integrator used is not `ImplicitEuler`.

Using a Laplacian, e.g. something like [Diffusion.md], for the displacement
fields will smooth the interior mesh as the mesh is deformed through this
boundary condition. However, it is still possible to run into poor aspect ratio
elements, or if you have more tangential displacement at one boundary node than
another then you can get inverted elements.

!syntax parameters /BCs/INSADDisplaceBoundaryBC

!syntax inputs /BCs/INSADDisplaceBoundaryBC

!syntax children /BCs/INSADDisplaceBoundaryBC

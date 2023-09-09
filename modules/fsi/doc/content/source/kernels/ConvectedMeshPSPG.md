# ConvectedMeshPSPG

!syntax description /Kernels/ConvectedMeshPSPG

This object adds the pressure-stabilized Petrov-Galerkin term to the pressure
equation corresponding to the [ConvectedMesh.md] object. It implements the
weak form

\begin{equation}
\int \nabla \psi \dot \left(\tau \dot{\vec{d}}\right) dV
\end{equation}

where $\nabla \psi$ is the gradient of the pressure test function, $\tau$ is a
stabilization parameter computed automatically by the `navier_stokes` base class
`INSBase`, and $\dot{\vec{d}}$ is the time derivative of the displacement
vector.

!syntax parameters /Kernels/ConvectedMeshPSPG

!syntax inputs /Kernels/ConvectedMeshPSPG

!syntax children /Kernels/ConvectedMeshPSPG

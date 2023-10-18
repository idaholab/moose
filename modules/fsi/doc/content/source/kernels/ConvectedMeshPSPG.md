# ConvectedMeshPSPG

!syntax description /Kernels/ConvectedMeshPSPG

This object adds the pressure-stabilized Petrov-Galerkin term to the pressure
equation corresponding to the [ConvectedMesh.md] object. It implements the
weak form

\begin{equation}
\label{pspg}
\int \nabla \psi \cdot \left(\tau\frac{\partial\vec{d_m}}{\partial t}\cdot\nabla\vec{u}\right) dV
\end{equation}

where $\nabla \psi$ is the gradient of the pressure test function, $\tau$ is a
stabilization parameter computed automatically by the `navier_stokes` base class
`INSBase`, $\vec{d_m}$ is the fluid mesh displacements, and
$\vec{u}$ is the fluid velocity. Note that when comparing [pspg] with
[!eqref](ConvectedMesh.md#convection) that the minus sign and density $\rho$ have
disappeared. This is because the form of PSPG stabilization is

\begin{equation}
\int \nabla \psi \cdot \left(-\frac{\tau}{\rho}\vec{R}\right)
\end{equation}

where $\vec{R}$ denotes the strong form of the momentum residuals.

!syntax parameters /Kernels/ConvectedMeshPSPG

!syntax inputs /Kernels/ConvectedMeshPSPG

!syntax children /Kernels/ConvectedMeshPSPG

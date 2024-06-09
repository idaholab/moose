# INSADMomentumMeshAdvection

!syntax description /Kernels/INSADMomentumMeshAdvection

`INSADMomentumMeshAdvection` implements the corresponding weak form for the components of
the term representing the contribution of the mesh displacement to the conservation of momentum:

\begin{equation}
-\rho \left(\frac{\partial\vec{d_m}}{\partial t} \cdot \nabla\right) \vec{u}
\end{equation}

where $\rho$ is the density, $\vec{d_m}$ is the fluid mesh displacements, and
$\vec{u}$ is the fluid velocity. This term is essential for obtaining the
correct convective derivative of the fluid in cases where the fluid mesh is
dynamic, e.g. in simulations of fluid-structure interaction or Arbitrary
Eulerian Lagrangian (ALE) simulations.

!syntax parameters /Kernels/INSADMomentumMeshAdvection

!syntax inputs /Kernels/INSADMomentumMeshAdvection

!syntax children /Kernels/INSADMomentumMeshAdvection

!tag name=INSADMomentumMeshAdvection pairs=module:navier_stokes system:kernels

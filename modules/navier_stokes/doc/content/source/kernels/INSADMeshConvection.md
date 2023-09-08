# INSADMeshConvection

!syntax description /Kernels/INSADMeshConvection

`INSADMeshConvection` implements the corresponding weak form for the components of
the term:

\begin{equation}
-\rho \left(\frac{\partial\vec{d_m}}{\partial t} \cdot \nabla\right) \vec{u}
\end{equation}

where $\rho$ is the density, $\vec{d_m}$ is the fluid mesh displacements, and
$\vec{u}$ is the fluid velocity. This term is essential for obtaining the
correct convective derivative of the fluid in cases where the fluid mesh is
dynamic, e.g. in simulations of fluid-structure interaction.

!syntax parameters /Kernels/INSADMeshConvection

!syntax inputs /Kernels/INSADMeshConvection

!syntax children /Kernels/INSADMeshConvection

# INSADMassPSPG

This object adds stabilization to the incompressible mass continuity equation in
the form of $\left(\nabla\psi_i, -\frac{\tau}{\rho}\vec R\right)$ where $\tau$
is the stabilization parameter, $\rho$ is the density and $\vec R$ is the strong
residual of the momentum equation. This is stabilizing because $\vec R$ includes
a pressure gradient term, producing a diffusion term like so:
$\left(\nabla\psi_i, \nabla p\right)$. Because the mass equation now has an
on-diagonal dependence, the saddle point nature of the incompressible equations
is removed and equal order shape functions can be used for velocity and pressure
variables.

!syntax description /Kernels/INSADMassPSPG

!syntax parameters /Kernels/INSADMassPSPG

!syntax inputs /Kernels/INSADMassPSPG

!syntax children /Kernels/INSADMassPSPG

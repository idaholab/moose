# NavierStokesHDGKernel

This class implements the steady incompressible Navier-Stokes equations. These include the conservation of mass:

!equation id=mass-eq
\nabla \cdot \vec{u} = 0

where $\vec{u}$ is the velocity and conservation of momentum:

!equation id=momentum-eq
\nabla \cdot \left(\rho \vec{u} \otimes \vec{u}\right) - \nabla \cdot \left(\mu
\nabla\vec{u}\right) + \nabla p = 0

where $\rho$ is the density, $\mu$ is the dynamic viscosity and $p$ is the pressure. This class uses
the hybridization laid out in [!citep](nguyen2011implicit). Note that, as shown
in the reference, the pressure field is integrated by parts which has
consequences for boundary conditions on momentum flux boundaries.

!syntax parameters /HDGKernels/NavierStokesHDGKernel

!syntax inputs /HDGKernels/NavierStokesHDGKernel

!syntax children /HDGKernels/NavierStokesHDGKernel

# NavierStokesHybridizedKernel

This class implements the steady incompressible Navier-Stokes equations. These include the conservation of mass:

!equation id=mass-eq
\nabla \cdot \vec{u} = 0

where $\vec{u}$ is the velocity and conservation of momentum:

!equation id=momentum-eq
\nabla \cdot \left(\vec{u} \otimes \vec{u}\right) - \nabla \cdot \left(\nu
\nabla\vec{u}\right) + \nabla p = 0

where $\nu$ is the kinematic viscosity and $p$ is the pressure. This class uses
the hybridization laid out in [!citep](nguyen2011implicit).

!syntax parameters /HybridizedKernels/NavierStokesHybridizedKernel

!syntax inputs /HybridizedKernels/NavierStokesHybridizedKernel

!syntax children /HybridizedKernels/NavierStokesHybridizedKernel

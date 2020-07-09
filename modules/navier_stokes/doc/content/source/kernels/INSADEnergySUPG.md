# INSADEnergySUPG

This object adds stabilization to the incompressible energy equation in
the form of $\left(\tau \vec u \cdot \nabla\psi_i, \vec R\right)$ where $\tau$
is the stabilization parameter, $\vec u$ is the velocity vector and $\vec R$ is the strong
residual of the momentum equation. $\tau$ is calculated based on the advection velocity, thermal
conductivity, density, heat capacity, time step size, and mesh size.  This term adds additional (consistent)
streamline diffusion such that higher Reynolds numbers can be simulated without
producing crippling oscillations.

!syntax description /Kernels/INSADEnergySUPG

!syntax parameters /Kernels/INSADEnergySUPG

!syntax inputs /Kernels/INSADEnergySUPG

!syntax children /Kernels/INSADEnergySUPG

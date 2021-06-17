# INSFVMomentumTimeDerivative

This object adds the $\rho\frac{\partial \vec u_i}{\partial t}$ term of the
incompressible Navier Stokes momentum equation where $\rho$ is the density,
$\vec u_i$ refers to the i'th component of the velocity $\vec{u}$, and $t$ is
time. This kernel must be applied for every component of the velocity.

!syntax parameters /FVKernels/INSFVMomentumTimeDerivative

!syntax inputs /FVKernels/INSFVMomentumTimeDerivative

!syntax children /FVKernels/INSFVMomentumTimeDerivative

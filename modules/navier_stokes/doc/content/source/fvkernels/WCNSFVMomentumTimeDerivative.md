# WCNSFVMomentumTimeDerivative

This object adds the $\rho\frac{\partial \vec u_i}{\partial t} + \frac{\partial \rho}{\partial t} \vec u_i $ term of the
weakly compressible Navier Stokes momentum equation where $\rho$ is the density,
$\vec u_i$ refers to the i'th component of the velocity $\vec{u}$, and $t$ is
time. This kernel must be applied for every component of the velocity.

!syntax parameters /FVKernels/WCNSFVMomentumTimeDerivative

!syntax inputs /FVKernels/WCNSFVMomentumTimeDerivative

!syntax children /FVKernels/WCNSFVMomentumTimeDerivative

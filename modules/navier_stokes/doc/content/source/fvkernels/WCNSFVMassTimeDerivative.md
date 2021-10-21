# WCNSFVMassTimeDerivative

This object adds the $\frac{\partial \rho}{\partial t}$ term of the
weakly compressible Navier Stokes mass equation where $\rho$ is the density,
and $t$ is time. This kernel is applied along with a [INSFVMassAdvection.md] kernel
to form the conservation of mass equation.

!syntax parameters /FVKernels/WCNSFVMassTimeDerivative

!syntax inputs /FVKernels/WCNSFVMassTimeDerivative

!syntax children /FVKernels/WCNSFVMassTimeDerivative

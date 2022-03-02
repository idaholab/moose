# PWCNSFVMassTimeDerivative

This object adds the $\frac{\partial \epsilon \rho}{\partial t}$ term of the
weakly compressible porous medium Navier Stokes mass equation where $\epsilon$ denotes
the fluid fraction, $\rho$ the density, and $t$ the time.
This kernel is applied along with a [INSFVMassAdvection.md] kernel
to form the conservation of mass equation.

!syntax parameters /FVKernels/PWCNSFVMassTimeDerivative

!syntax inputs /FVKernels/PWCNSFVMassTimeDerivative

!syntax children /FVKernels/PWCNSFVMassTimeDerivative

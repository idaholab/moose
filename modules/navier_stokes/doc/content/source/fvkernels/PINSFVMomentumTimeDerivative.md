# PINSFVMomentumTimeDerivative

This object adds the $\rho\frac{\partial \vec u_{di}}{\partial t}$ term of the
porous media incompressible Navier Stokes momentum equation where $\rho$ is the density,
$\vec u_{di}$ refers to the i'th component of the superficial velocity $\vec{ud}$, and $t$ is
time. This kernel must be applied for every component of the velocity.

!syntax parameters /FVKernels/PINSFVMomentumTimeDerivative

!syntax inputs /FVKernels/PINSFVMomentumTimeDerivative

!syntax children /FVKernels/PINSFVMomentumTimeDerivative

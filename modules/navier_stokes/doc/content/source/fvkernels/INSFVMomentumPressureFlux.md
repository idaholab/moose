# INSFVMomentumPressureFlux

This object adds the $\nabla p$ pressure gradient term of the incompressible
Navier Stokes momentum equation as a surface term using the divergence theorem.

The pressure is a Lagrange Multiplier that ensures the incompressibility constraint.

!syntax parameters /FVKernels/INSFVMomentumPressureFlux

!syntax inputs /FVKernels/INSFVMomentumPressureFlux

!syntax children /FVKernels/INSFVMomentumPressureFlux

!tag name=INSFVMomentumPressureFlux pairs=module:navier_stokes system:fvkernels

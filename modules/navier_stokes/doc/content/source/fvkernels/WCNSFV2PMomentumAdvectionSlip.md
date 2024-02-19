# WCNSFV2PMomentumAdvectionSlip

This object implements the $\nabla \cdot \left(\rho\vec u_{slip} \otimes \vec u_{slip} \right)$
component terms of the mixture model in the two-phase Navier Stokes momentum equation.
The term is added to the Rhie-Chow interpolation in a similar way than the momentum advection
kernel. See [INSFVMomentumAdvection.md] for more details.

!syntax parameters /FVKernels/WCNSFV2PMomentumAdvectionSlip

!syntax inputs /FVKernels/WCNSFV2PMomentumAdvectionSlip

!syntax children /FVKernels/WCNSFV2PMomentumAdvectionSlip

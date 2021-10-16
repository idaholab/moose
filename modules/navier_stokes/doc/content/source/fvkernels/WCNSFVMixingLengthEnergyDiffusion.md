# WCNSFVMixingLengthEnergyDiffusion

This kernel uses a mixing-length model to compute the turbulent diffusion of energy,
$\nabla \cdot \overline{ c' \vec T}$, which appears in
Reynolds-averaged conservation equations.

where c' is the ratio

!equation
c' = \dfrac{\rho c_p \nu_t}{Pr_t}

with $\rho$ the fluid density, $c_p$ the specific heat capacity, $\nu_t$ the turbulent kinematic viscosity
and $Pr_t$ the turbulent Prandtl number.

!syntax parameters /FVKernels/WCNSFVMixingLengthEnergyDiffusion

!syntax inputs /FVKernels/WCNSFVMixingLengthEnergyDiffusion

!syntax children /FVKernels/WCNSFVMixingLengthEnergyDiffusion

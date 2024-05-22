# Navier Stokes Solid Heat Transfer / PNSFVSolidHeatTransfer

!syntax description /Physics/NavierStokes/SolidHeatTransfer/PNSFVSolidHeatTransferPhysics

## Equation

This [Physics](Physics/index.md) object creates the kernels and boundary conditions to solve the heat transfer
equation for the solid phase in a porous media flow problem.

!equation
\dfrac{\partial (1-\epsilon) \rho h_s}{\partial t} - \nabla \cdot (\kappa_s \nabla T_s) - \alpha (T_f - T_s) - (1-\epsilon) \dot{Q} = 0

where:

- $h_s$ is the solid specific enthalpy, computed from the specific heat $c_{ps}$ and the solid temperature
- $\rho$ is the solid density
- $\epsilon$ is the porosity
- $T_s$ is the solid temperature
- $T_f$ is the fluid temperature
- $k_s$ the solid thermal conductivity
- $(1-\epsilon) Q$ is the source term, corresponding to energy deposited directly in the solid phase
- $\alpha$ is the ambient convection volumetric heat transfer coefficient

The enthalpy is used in lieu of $\rho c_p T$ to be able to model solids with temperature dependent
specific heat.

The kernels potentially created for this equation are:

- [INSFVEnergyTimeDerivative.md] for the time derivative of the energy
- [PINSFVEnergyDiffusion.md] for energy diffusion with an isotropic thermal diffusivity
- [PINSFVEnergyAnisotropicDiffusion.md] for energy diffusion with an anisotropic thermal diffusivity
- [FVCoupledForce.md] for the energy source term
- [PINSFVEnergyAmbientConvection.md] for the volumetric ambient convection term, if present

!alert note
Additional details on porous media flow equations can be found on this [page](navier_stokes/pinsfv.md).

## Automatically created variables

The `PNSFVSolidHeatTransferPhysics` by default will automatically create a nonlinear variable
for the solid phase temperature. It should be named as follows:

!listing include/base/NS.h line=std::string T_solid

## Coupling with other Physics

In the presence of fluid flow, a [WCNSFVFluidHeatTransferPhysics.md] should be created
using the `[Physics/NavierStokes/FluidHeatTransfer/<name>]` syntax. The following input performs
the coupling between the fluid equations and the solid temperature equations. The coupling
between the fluid and solid domain is performed through a volumetric ambient convection term.

!listing test/tests/finite_volume/pwcns/channel-flow/2d-transient-physics.i block=Physics

!alert warning
Conjugate heat transfer on a surface on the boundary of the fluid domain is not currently implemented
with the `Physics` syntax. Please use a [FVConvectionCorrelationInterface.md] for that purpose.

!syntax parameters /Physics/NavierStokes/SolidHeatTransfer/PNSFVSolidHeatTransferPhysics

!syntax inputs /Physics/NavierStokes/SolidHeatTransfer/PNSFVSolidHeatTransferPhysics

!syntax children /Physics/NavierStokes/SolidHeatTransfer/PNSFVSolidHeatTransferPhysics

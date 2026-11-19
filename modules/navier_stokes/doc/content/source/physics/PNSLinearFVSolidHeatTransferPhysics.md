# Navier Stokes Solid Heat Transfer Segregated / PNSLinearFVSolidHeatTransferPhysics

!syntax description /Physics/NavierStokes/SolidHeatTransferSegregated/PNSLinearFVSolidHeatTransferPhysics

## Equation

This [Physics](Physics/index.md) object creates the kernels and boundary conditions to solve the
energy equation of the solid phase of a porous medium with the linear finite volume
discretization, for the segregated solvers such as [SIMPLE.md]:

!equation
(1-\epsilon) \rho_s c_{p,s} \dfrac{\partial T_s}{\partial t} - \nabla \cdot (\kappa_s \nabla T_s) - \alpha (T_f - T_s) - \dot{Q} = 0

where:

- $\epsilon$ is the porosity
- $\rho_s$ is the solid density
- $c_{p,s}$ is the solid specific heat
- $T_s$ is the solid temperature
- $T_f$ is the fluid temperature
- $\kappa_s$ is the effective solid thermal conductivity, which accounts for the solid fraction
- $\alpha$ is the ambient convection volumetric heat transfer coefficient
- $\dot{Q}$ is the heat source deposited in the solid, per unit total volume

The solid is stationary, so the porosity and the solid density must be constant in time. The
specific heat may depend on the temperature: since $dh_s = c_{p,s} dT_s$, the time derivative above
is the rate of change of the solid enthalpy $(1-\epsilon) \rho_s h_s$.

The parameters are the same as [PNSFVSolidHeatTransferPhysics.md], to facilitate the transition
between discretizations. The kernels created are:

- [LinearFVTimeDerivative.md] for the time derivative
- [LinearFVDiffusion.md] for the conduction term, or [LinearFVAnisotropicDiffusion.md] if the
  solid thermal conductivity is a vector functor, the diagonal of an anisotropic conductivity
  tensor. Heat flux boundaries then use [LinearFVAnisotropicDiffusionFunctorNeumannBC.md].
- [LinearFVVolumetricHeatTransfer.md] for the heat exchange with the fluid, if present
- [LinearFVSource.md] for the heat source, if present

Faces of the solid domain without a boundary condition are insulated.

## Automatically created variables

The `PNSLinearFVSolidHeatTransferPhysics` creates a linear finite volume variable for the solid
temperature, named by default:

!listing include/base/NS.h line=std::string T_solid

## Coupling with other Physics

The fluid phase is solved by the [WCNSLinearFVFlowPhysics.md] with the porous medium treatment and
the [WCNSLinearFVFluidHeatTransferPhysics.md]. The volumetric heat exchange between the phases is
defined in both Physics with the same heat transfer coefficients. The following input couples the
fluid and solid energy equations in a porous block bounded by two pressure baffles (porosity change
interfaces):

!listing test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-simple-channel-enthalpy/porous-baffle-2d-enthalpy-physics.i block=Physics

!syntax parameters /Physics/NavierStokes/SolidHeatTransferSegregated/PNSLinearFVSolidHeatTransferPhysics

!syntax inputs /Physics/NavierStokes/SolidHeatTransferSegregated/PNSLinearFVSolidHeatTransferPhysics

!syntax children /Physics/NavierStokes/SolidHeatTransferSegregated/PNSLinearFVSolidHeatTransferPhysics

# PWCNSFVMomentumFluxBC

!syntax description /FVBCs/PWCNSFVMomentumFluxBC

This object is the porous medium version of [WCNSFVMomentumFluxBC.md].

The momentum flux is:

!equation
\phi = \dfrac{\rho v_d^2}{\epsilon} = \dfrac{\dot{m}^2}{\rho \epsilon A^2}

with $\phi$ the momentum flux, $\rho$ the density, $v_d$ the fluid superficial
velocity (assumed normal to the surface here), $\epsilon$ the porosity of the medium,
$\dot{m}$ the mass flow rate, and $A$ the inlet area.

There are two options for specifying the momentum flux:

- specifying a mass flow rate postprocessor, which is then divided by the area of the inlet,
  which may also be a postprocessor.

- specifying an inlet superficial velocity postprocessor and a density functor. The functor is
  usually a functor material property, defined by a [GeneralFunctorFluidProps.md].

The scaling factor may be used if the inlet is not aligned with the X or Y direction,
in which case a projection is necessary and this boundary condition should be used for
both components of the momentum equation.

This boundary condition works with postprocessors, which may be replaced by constant
values in the input. The intended use case for this boundary condition is to be receiving its value from
a coupled application, using a [Receiver.md] postprocessor.

!syntax parameters /FVBCs/PWCNSFVMomentumFluxBC

!syntax inputs /FVBCs/PWCNSFVMomentumFluxBC

!syntax children /FVBCs/PWCNSFVMomentumFluxBC

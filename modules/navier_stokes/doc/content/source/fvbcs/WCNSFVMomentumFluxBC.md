# WCNSFVMomentumFluxBC

!syntax description /FVBCs/WCNSFVMomentumFluxBC

The momentum flux is:

!equation
\phi = \rho v^2 = \dfrac{\dot{m}^2}{\rho A^2}

with $\phi$ the momentum flux, $\rho$ the density, $v$ the fluid velocity (assumed normal to the surface),
$\dot{m}$ the mass flow rate and $A$ the inlet area.

There are two options for specifying the momentum flux:

- specifying a mass flow rate postprocessor, which is then divided by the area of the inlet,
  which may also be a postprocessor.

- specifying an inlet velocity postprocessor and a density functor. The functor is
  usually a functor material property, defined by a [GeneralFunctorFluidProps.md].


The scaling factor may be used if the inlet is not aligned with the X or Y direction,
in which case a projection is necessary and this boundary condition should be used for
both components of the momentum equation.

This boundary condition works with postprocessors, which may be replaced by constant
values in the input. The intended use case for this boundary condition is to be receiving its value from
a coupled application, using a [Receiver.md] postprocessor.

## Example input syntax

In this example input, the inlet boundary condition to the momentum conservation equation is
specified using a `WCNSFVMomentumFluxBC`. The momentum flux is specified using the mass flow rate
and the inlet area.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/boundary_conditions/flux_bcs_mdot.i block=FVBCs

!syntax parameters /FVBCs/WCNSFVMomentumFluxBC

!syntax inputs /FVBCs/WCNSFVMomentumFluxBC

!syntax children /FVBCs/WCNSFVMomentumFluxBC

# WCNSFVMassFluxBC

!syntax description /FVBCs/WCNSFVMassFluxBC

This boundary condition works with postprocessors, which can be replaced by constant
values. The intended use case for this boundary condition is to be receiving its value from
a coupled application, using a [Receiver.md] postprocessor.

The mass flux is:

!equation
\phi = \rho v = \dfrac{\dot{m}}{A}

with $\phi$ the mass flux, $\rho$ the density, $\v$ the fluid velocity,
$\dot{m}$ the mass flow rate and $A$ the inlet area.

There are two options for specifying the mass flux:

- specifying a mass flow rate postprocessor, which is then divided by the area of the inlet,
  which may also be a postprocessor.

- specifying an inlet velocity postprocessor and a density functor. The functor is
  usually a functor material property, defined by a [GeneralFunctorFluidProps.md].


## Example input syntax

In this example input, the boundary conditions to the mass conservation equation are
specified using a `WCNSFVMassFluxBC`. The mass flux is specified using the mass flow rate
and the inlet area.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/fvbcs/flux_bcs_mdot.i block=FVBCs

!syntax parameters /FVBCs/WCNSFVMassFluxBC

!syntax inputs /FVBCs/WCNSFVMassFluxBC

!syntax children /FVBCs/WCNSFVMassFluxBC

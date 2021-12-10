# WCNSFVEnergyFluxBC

!syntax description /FVBCs/WCNSFVEnergyFluxBC

The energy flux is:

!equation
\phi = v \rho c_p T = \dfrac{\dot{e}}{A} = \dfrac{\dot{m} c_p T}{A}

with $\phi$ the energy flux, $\rho$ the density, $c_p$ the specific heat capacity, $\v$ the fluid velocity (assumed normal to the surface),
$T$ the fluid temperature, $\dot{e}$ the energy flow rate, $\dot{m}$ the mass flow rate and $A$ the inlet area.

There are three options for specifying the energy flux:

- specifying an energy flow rate postprocessor, which is then divided by the area of the inlet,
  which may also be a postprocessor.

- specifying an inlet velocity and an inlet temperature postprocessors, a specific heat capacity and a density functor.

- specifying an inlet mass flow rate, a specific heat capacity and a n inlet temperature postprocessor.


The functors needed are usually functor material properties, defined by a [GeneralFunctorFluidProps.md].

This boundary condition works with postprocessors, which may be replaced by constant
values in the input. The intended use case for this boundary condition is to be receiving its value from
a coupled application, using a [Receiver.md] postprocessor.

## Example input syntax

In this example input, the inlet boundary condition to the energy conservation equation is
specified using a `WCNSFVEnergyFluxBC`. The energy flux is specified using the mass flow rate, the inlet area,
the specific heat capacity and the temperature.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/boundary_conditions/flux_bcs_mdot.i block=FVBCs

!syntax parameters /FVBCs/WCNSFVEnergyFluxBC

!syntax inputs /FVBCs/WCNSFVEnergyFluxBC

!syntax children /FVBCs/WCNSFVEnergyFluxBC

# WCNSFVInletTemperatureBC

!syntax description /FVBCs/WCNSFVInletTemperatureBC

There are three options for specifying the inlet temperature of the system:

- specifying a temperature postprocessor

- specifying an energy flow rate postprocessor, a mass flow rate postprocessor and a specific heat
  capacity functor. The functors are usually a functor material property, defined by a [GeneralFunctorFluidProps.md].
  The scaling factor can be used to account for projections if the inlet flow and
  the surface are not aligned.

- specifying an energy flow rate postprocessor, a mass flow rate postprocessor, a specific heat capacity
  and a density functor. The functors are usually a functor material property, defined by a [GeneralFunctorFluidProps.md].
  The scaling factor can be used to account for projections if the inlet flow and
  the surface are not aligned.


This boundary condition works with postprocessors, which may be replaced by constant
values in the input. The intended use case for this boundary condition is to be receiving its value from
a coupled application, using a [Receiver.md] postprocessor.

!alert note
Specifying the inlet temperature using a `WCNSFVInletTemperatureBC` will not preserve
energy flow at the boundary in most cases, in part because of the discretization error.
Specifying an incoming energy flux using a [WCNSFVEnergyFluxBC.md] is currently the only conservative
approach.

## Example input syntax

In this example input, the inlet boundary condition to the energy conservation equation
is specified using two `WCNSFVInletTemperatureBC`.
The inlet temperature is specified using an energy and a mass flow rate postprocessors.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/boundary_conditions/dirichlet_bcs_mdot.i block=FVBCs

!syntax parameters /FVBCs/WCNSFVInletTemperatureBC

!syntax inputs /FVBCs/WCNSFVInletTemperatureBC

!syntax children /FVBCs/WCNSFVInletTemperatureBC

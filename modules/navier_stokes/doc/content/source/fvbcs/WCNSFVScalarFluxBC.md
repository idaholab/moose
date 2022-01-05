# WCNSFVScalarFluxBC

!syntax description /FVBCs/WCNSFVScalarFluxBC

The flux for scalar quantity $c$ is:

!equation
\phi = v c = \dfrac{\dot{m} c}{\rho A}

with $\phi$ the scalar quantity flux, $\rho$ the density, $\v$ the fluid velocity,
$c$ the inlet value of the scalar quantity, $\dot{m}$ the mass flow rate and $A$ the inlet area.

There are three options for specifying the scalar flux:

- specifying the surface integrated scalar quantity flow rate directly, which is then divided
  by the area of the inlet to obtain the local flux.

- specifying an inlet velocity postprocessor and an inlet value for the scalar quantity.

- specifying the inlet value for the scalar quantity, a postprocessor for the mass flow rate,
  a density functor and the inlet surface area. These last three quantities are used to compute
  the inlet velocity.


This boundary condition works with postprocessors, which may be replaced by constant
values in the input. The intended use case for this boundary condition is to be receiving its value from
a coupled application, using a [Receiver.md] postprocessor.

## Example input syntax

In this example input, the inlet boundary conditions to the scalar quantity conservation equation is
specified using a `WCNSFVScalarFluxBC`. The scalar quantity flux is specified using the mass flow rate
and the inlet area.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/boundary_conditions/flux_bcs_mdot.i block=FVBCs

!syntax parameters /FVBCs/WCNSFVScalarFluxBC

!syntax inputs /FVBCs/WCNSFVScalarFluxBC

!syntax children /FVBCs/WCNSFVScalarFluxBC

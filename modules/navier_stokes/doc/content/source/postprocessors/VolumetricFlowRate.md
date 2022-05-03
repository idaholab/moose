# VolumetricFlowRate

!syntax description /Postprocessors/VolumetricFlowRate

## Example input syntax

In this example, we measure:

- the mass flow rate
- the momentum flow rate
- the enthalpy flow rate

with numerous `VolumetricFlowRate` postprocessors to prove the conservation of mass, momentum and energy
of the finite volume discretization of the incompressible Navier Stokes equations.

!listing test/tests/postprocessors/flow_rates/conservation_INSFV.i block=Postprocessors

!syntax parameters /Postprocessors/VolumetricFlowRate

!syntax inputs /Postprocessors/VolumetricFlowRate

!syntax children /Postprocessors/VolumetricFlowRate

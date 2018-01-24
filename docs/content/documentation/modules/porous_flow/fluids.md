# Using the Fluid Properties module in PorousFlow

PorousFlow can use any of the UserObjects in the [Fluid Properties](/fluid_properties/index.md) module that use the pressure-temperature formulation. These are used in the input file by adding the following block

!listing modules/porous_flow/test/tests/fluids/h2o.i block=Modules label=Water fluid properties

To calculate fluid properties, a
[`PorousFlowSingleComponentFluid`](/porous_flow/PorousFlowSingleComponentFluid.md)
`Material` can be used by adding

!listing modules/porous_flow/test/tests/fluids/h2o.i block=Materials label=Water fluid properties material

Fluid density, viscosity, internal energy and enthalpy are calculated by this
material (depending on the options chosen, see
[`PorousFlowSingleComponentFluid`](/porous_flow/PorousFlowSingleComponentFluid.md)
for details).

Multiple fluids can be included by simply adding additional fluids to the
`Modules` block and corresponding `PorousFlowSingleComponentFluid` entries in
the `Materials` block.

!!! note
    The fluid properties UserObjects expect temperature in Kelvin.

If the input file uses temperature in Celcius, the `temperature_unit` option in
[`PorousFlowSingleComponentFluid`](/porous_flow/PorousFlowSingleComponentFluid.md)
**must** be set to `Celcius`.

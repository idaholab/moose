# Using the Fluid Properties module in PorousFlow

PorousFlow can use any of the UserObjects in the [Fluid Properties](/fluid_properties/index.md)
module that use the pressure-temperature formulation. These are used in the input file by adding the
following block

!listing modules/porous_flow/test/tests/fluids/h2o.i block=Modules/FluidProperties

To calculate fluid properties, a
[`PorousFlowSingleComponentFluid`](/PorousFlowSingleComponentFluid.md)
`Material` can be used by adding

!listing modules/porous_flow/test/tests/fluids/h2o.i block=Materials

Fluid density, viscosity, internal energy and enthalpy are calculated by this material (depending on
the options chosen, see
[`PorousFlowSingleComponentFluid`](/PorousFlowSingleComponentFluid.md)
for details).

Multiple fluids can be included by simply adding additional fluids to the `Modules` block and
corresponding `PorousFlowSingleComponentFluid` entries in the `Materials` block.

!alert note
The fluid properties UserObjects expect temperature in Kelvin.

If the input file uses temperature in Celcius, the `temperature_unit` option in
[`PorousFlowSingleComponentFluid`](/PorousFlowSingleComponentFluid.md)
+must+ be set to `Celcius`.

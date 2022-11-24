# Fluid equations of state

PorousFlow uses formulations contained in the [Fluid Properties](/fluid_properties/index.md) module to
calculate fluid properties such as density or viscosity.

## Using the Fluid Properties module

### Implementation

PorousFlow can use any of the UserObjects in the [Fluid Properties](/fluid_properties/index.md)
module. A specific fluid can be included in the input file by adding the following block

!listing modules/porous_flow/test/tests/fluids/h2o.i block=FluidProperties

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

Due to this design, it is trivial to change the fluid in any simulation by simply swapping out the
[Fluid Properties](/fluid_properties/index.md) UserObjects.

!alert note
The fluid properties UserObjects expect temperature in Kelvin.

If the input file uses temperature in Celsius, the `temperature_unit` option in
[`PorousFlowSingleComponentFluid`](/PorousFlowSingleComponentFluid.md)
+must+ be set to `Celsius`.

### Performance

Computing fluid properties such as density and viscosity can be expensive when using
detailed fluid equations of state (such as [Water97FluidProperties](/Water97FluidProperties.md) or
[CO2FluidProperties](/CO2FluidProperties.md)). In some cases, these computations can be a significant
proportion of the overall time taken.

In order to reduce the computational expense of using high-precision fluid formulations, the
[Fluid Properties](/fluid_properties/index.md) module provides a general UserObject that uses
bicubic interpolation to calculate fluid properties, see
[TabulatedFluidProperties](/TabulatedFluidProperties.md) for details. Using interpolation to
calculate fluid properties can significantly reduce the computational expense (particularly for
[CO2FluidProperties](/CO2FluidProperties.md), where the density is calculated iteratively from pressure
and temperature).

In the PorousFlow module, fluid density and viscosity are typically calculated at the same time. To
achieve the maximum benefit from [TabulatedFluidProperties](/TabulatedFluidProperties.md), users
should ensure that both density and viscosity are calculated by interpolation, by either providing
them in the supplied properties file, or making sure that both are specified in the
`interpolated_properties` input parameter if no data file exists.

### Available fluids

The full list of available fluids is provided in the [Fluid Properties](/fluid_properties/index.md) module.

# PorousFlowFluidPropertyIC

!syntax description /ICs/PorousFlowFluidPropertyIC

This initial condition provides a straightforward way to specify an initial value for a
fluid property using pressure and temperature as the inputs.

A valid [Fluid Properties](/fluid_properties/index.md) UserObject is required.

!alert note
The FluidProperties UserObject expects temperature in Kelvin. If the simulation uses temperature in Celsius, `temperature_units = celsius` must be used.

!syntax parameters /ICs/PorousFlowFluidPropertyIC

!syntax inputs /ICs/PorousFlowFluidPropertyIC

!syntax children /ICs/PorousFlowFluidPropertyIC

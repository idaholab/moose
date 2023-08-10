# ADHeatStructureHeatSource

!syntax description /Kernels/ADHeatStructureHeatSource

The equation term modeled by this kernel is:

!equation
\text{scale} \dfrac{\text{power} * \text{power shape}(x,t)}{N_{units} \text{power shape integral}}

where `power` is a scalar variable set using the [!param](/Kernels/ADHeatStructureHeatSource/total_power) parameter,
`power shape` is function set using the [!param](/Kernels/ADHeatStructureHeatSource/power_shape_function) parameter,
`scale` is a controllable scaling factor and $N_{units}$ is the number of heat structures modeled.

This kernel offers several options (scalar, scalar variable, function) to specify the value of the power. To use only one,
set all the others to `1`.

!alert note
If the function spatial shape is not integrated exactly by the quadrature, specifying the
[!param](/Kernels/ADHeatStructureHeatSource/power_shape_integral_pp) parameter will ensure that
power is conserved.

!alert note
In THM, most kernels are added automatically by components. This kernel is created by the
[HeatSourceFromTotalPower.md] component which is used to add heat sources to heat structures.

!syntax parameters /Kernels/ADHeatStructureHeatSource

!syntax inputs /Kernels/ADHeatStructureHeatSource

!syntax children /Kernels/ADHeatStructureHeatSource

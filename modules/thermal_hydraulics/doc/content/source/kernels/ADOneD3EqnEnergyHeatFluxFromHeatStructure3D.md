# ADOneD3EqnEnergyHeatFluxFromHeatStructure3D

!syntax description /Kernels/ADOneD3EqnEnergyHeatFluxFromHeatStructure3D

The heat flux to the flow channel is calculated using the average temperature on
the corresponding layer of the coupled boundary of a 3D heat structure.

The wall heating is expressed as a convection term in the energy equation strong form:

!equation
H_w P_{hf} (T_{fluid} - T_{wall})

where $H_w$ is the convective heat transfer coefficient, $P_{hf}$ is the heated perimeter,
$T_{fluid}$ is the single-phase fluid temperature and $T_{wall}$ is the wall temperature.
The wall temperature is computed by a [LayeredAverage.md] user object.

!alert note
In THM, most kernels are added automatically by components. This kernel is created by the
[ADOneD3EqnEnergyHeatFluxFromHeatStructure3D.md] for modeling the heat flux from the 3D heat structure.

!syntax parameters /Kernels/ADOneD3EqnEnergyHeatFluxFromHeatStructure3D

!syntax inputs /Kernels/ADOneD3EqnEnergyHeatFluxFromHeatStructure3D

!syntax children /Kernels/ADOneD3EqnEnergyHeatFluxFromHeatStructure3D

!bibtex bibliography

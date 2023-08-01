# OneD3EqnEnergyHeatSource

!syntax description /Kernels/OneD3EqnEnergyHeatSource

The local heat source in the energy equation strong form is:

!equation
q(\vec{x}, t) A

where $q$ is a volumetric heat source, a [Function](syntax/Functions/index.md) of space and time and $A$ is the local area of the component.

!alert warning
If the heat source function spatial shape is not integrated exactly by the quadrature, this will lead
to non-conservation of energy.

!alert note
In THM, most kernels are added automatically by components. This kernel is created to add volumetric heat sources
by the [HeatSourceVolumetric1Phase.md].

!syntax parameters /Kernels/OneD3EqnEnergyHeatSource

!syntax inputs /Kernels/OneD3EqnEnergyHeatSource

!syntax children /Kernels/OneD3EqnEnergyHeatSource

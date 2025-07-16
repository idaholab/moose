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
In THM, most kernels are added automatically by components. This kernel is no-longer in use, having
been replaced by its [AD](automatic_differentiation/index.md) counterpart [ADOneD3EqnEnergyHeatSource.md],
designed to provide numerically exact contributions to the Jacobian.

!syntax parameters /Kernels/OneD3EqnEnergyHeatSource

!syntax inputs /Kernels/OneD3EqnEnergyHeatSource

!syntax children /Kernels/OneD3EqnEnergyHeatSource

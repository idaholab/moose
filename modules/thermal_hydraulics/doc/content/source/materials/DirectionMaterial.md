# DirectionMaterial

!syntax description /Materials/DirectionMaterial

This material declares the "direction" material property.

This material is added automatically for every `FlowChannelBase` derived component, which includes most
flow components.

## Example input syntax

In this example, the `DirectionMaterial` is used to compute the direction of the elements,
and the "direction" is passed as a parameter to the `energy_flux` kernel.

!listing tests/jacobians/kernels/one_d_3eqn_energy_flux.i block=Materials Kernels

!syntax parameters /Materials/DirectionMaterial

!syntax inputs /Materials/DirectionMaterial

!syntax children /Materials/DirectionMaterial

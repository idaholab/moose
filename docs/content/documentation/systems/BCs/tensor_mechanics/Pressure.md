# Pressure
!syntax description /BCs/Pressure

## Description
The boundary condition, `Pressure` applies a force to a mesh boundary in the magnitude specified by the user.
A `component` of the normal vector to the mesh surface (0, 1, or 2 corresponding to the $\hat{x}$, $\hat{y}$, and $\hat{z}$ vector components) is used to determine the direction in which to apply the traction.
The boundary condition is always applied to the displaced mesh.

The magnitude of the `Pressure` boundary condition can be specified as either a scalar (use the input parameter `factor`), a `function` parameter, or a `Postprocessor` name.

A set of `Pressure` boundary conditions applied to multiple variables in multiple components can be defined with the [PressureAction](/BCs/Pressure/tensor_mechanics/PressureAction.md).

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/1D_spherical/finiteStrain_1DSphere_hollow.i block=BCs/outerPressure

!syntax parameters /BCs/Pressure

!syntax inputs /BCs/Pressure

!syntax children /BCs/Pressure

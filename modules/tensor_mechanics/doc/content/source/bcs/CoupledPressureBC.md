# Coupled Pressure BC

## Description

The boundary condition `CoupledPressureBC` applies a force computed in a variable to a mesh boundary.
A `component` of the normal vector to the mesh surface (0, 1, or 2 corresponding to the $\hat{x}$, $\hat{y}$, and $\hat{z}$ vector components) is used to determine the direction in which to apply the traction.

!alert note
The boundary condition is always applied to the displaced mesh.

The `CoupledPressureBC` is typically used in a multi-app scenario.
The pressure variable can be computed by a sub-app (it can be for example a flow code) and then transferred into an auxiliary variable, which is then coupled into this boundary condition so that it is applied in the master app.

A set of `CoupledPressure` boundary conditions applied to multiple variables in multiple components can be defined with the [CoupledPressureAction](/CoupledPressureAction.md).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/coupled_pressure/coupled_pressure_test.i block=BCs/side3_x

!syntax parameters /BCs/CoupledPressureBC

!syntax inputs /BCs/CoupledPressureBC

!syntax children /BCs/CoupledPressureBC

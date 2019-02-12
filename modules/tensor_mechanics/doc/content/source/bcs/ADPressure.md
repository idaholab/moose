# ADPressure

!syntax description /ADBCs/ADPressure<RESIDUAL>

## Description

The boundary condition, `ADPressure` applies a force to a mesh boundary in the
magnitude specified by the user. A `component` of the normal vector to the mesh
surface (0, 1, or 2 corresponding to the $\hat{x}$, $\hat{y}$, and $\hat{z}$
vector components) is used to determine the direction in which to apply the
traction. The boundary condition is always applied to the displaced mesh and
uses forward mode automatic differentiation to compute an exact Jacobian
contribution (this is contingent on coupling only AD enabled objects in the
parameters).

The magnitude of the `ADPressure` boundary condition can be specified as either
a constant scalar factor (use the input parameter `constant`), a factor from a
`function`, a factor from a `postprocessor`, or any combination thereof.

!syntax parameters /ADBCs/ADPressure<RESIDUAL>

!syntax inputs /ADBCs/ADPressure<RESIDUAL>

!syntax children /ADBCs/ADPressure<RESIDUAL>

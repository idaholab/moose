# XFEMPressure

!syntax description /DiracKernels/XFEMPressure

## Overview

`XFEMPressure` applies a force to an interface cut by XFEM in the magnitude specified by the user. This is implemented with DiracKernel. A `component` of the normal vector to the interface (0, 1, or 2 corresponding
to the $\hat{x}$, $\hat{y}$, and $\hat{z}$ vector components) is used to determine
the direction in which to apply the traction.

- For the variable `disp_x`, the parameter `component` sets to be 0
- For the variable `disp_y`, the parameter `component` sets to be 1
- For the variable `disp_z`, the parameter `component` sets to be 2

The magnitude of the `Pressure` boundary condition can be specified as either a
scalar (use the input parameter `factor`) or a `function` parameter.

## Example Input Syntax

!listing test/tests/pressure_bc/edge_2d_pressure.i block=DiracKernels

!syntax parameters /DiracKernels/XFEMPressure

!syntax inputs /DiracKernels/XFEMPressure

!syntax children /DiracKernels/XFEMPressure

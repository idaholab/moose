# ADFunctionPresetBC

!syntax description /BCs/ADFunctionPresetBC

## Description

The `ADFunctionPresetBC` takes the same inputs as
[ADFunctionDirichletBC](/ADFunctionDirichletBC.md) and also acts as a
Dirichlet boundary condition.  However, the implementation is slightly different;
`ADFunctionPresetBC` causes the value of the boundary condition to be applied before the
solve begins where [ADFunctionDirichletBC](/ADFunctionDirichletBC.md)
enforces the boundary condition as the solve
progresses.  In certain situations, one is better than another.

## Example Input Syntax

!listing test/tests/bcs/bc_preset_nodal/ad-bc_function_preset.i block=BCs

!syntax parameters /BCs/ADFunctionPresetBC

!syntax inputs /BCs/ADFunctionPresetBC

!syntax children /BCs/ADFunctionPresetBC

# FunctionPresetBC

!syntax description /BCs/FunctionPresetBC

## Description

The `FunctionPresetBC` takes the same inputs as
[FunctionDirichletBC](/FunctionDirichletBC.md) and also acts as a
Dirichlet boundary condition.  However, the implementation is slightly different;
`FunctionPresetBC` causes the value of the boundary condition to be applied before the
solve begins where [FunctionDirichletBC](/FunctionDirichletBC.md)
enforces the boundary condition as the solve
progresses.  In certain situations, one is better than another.

## Example Input Syntax

!listing test/tests/bcs/bc_preset_nodal/bc_function_preset.i block=BCs

!syntax parameters /BCs/FunctionPresetBC

!syntax inputs /BCs/FunctionPresetBC

!syntax children /BCs/FunctionPresetBC

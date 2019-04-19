# ADPresetBC

!syntax description /BCs/ADPresetBC

## Description

The `ADPresetBC` takes the same inputs as [DirichletBC](/DirichletBC.md)
and also acts as a Dirichlet boundary condition.  However, the implementation is slightly different;
`PresetBC` causes the value of the boundary condition to be applied before the solve begins where
[DirichletBC](/DirichletBC.md) enforces the boundary condition as the solve progresses.  In
certain situations, one is better than another.

## Example Input Syntax

!listing test/tests/bcs/bc_preset_nodal/ad-bc_preset_nodal.i block=BCs

!syntax parameters /BCs/ADPresetBC

!syntax inputs /BCs/ADPresetBC

!syntax children /BCs/ADPresetBC

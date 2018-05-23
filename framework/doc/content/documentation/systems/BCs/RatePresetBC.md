# RatePresetBC

!syntax description /BCs/RatePresetBC

## Description

The `RatePresetBC` takes the same inputs as
[RateDirichletBC](/BCs/RateDirichletBC.md) and also acts as a
Dirichlet boundary condition.  However, the implementation is slightly different.
`RatePresetBC` causes the value of the boundary condition to be applied before the
solve begins where [RateDirichletBC](/BCs/RateDirichletBC.md)
enforces the boundary condition as the solve
progresses.  In certain situations, one is better than another.

## Example Input Syntax

!listing test/tests/bcs/rate_bcs/bc_rate_preset.i start=[./right_rate] end=[../] include-end=true

!syntax parameters /BCs/RatePresetBC

!syntax inputs /BCs/RatePresetBC

!syntax children /BCs/RatePresetBC

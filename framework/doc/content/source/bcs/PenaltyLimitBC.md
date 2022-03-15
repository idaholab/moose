# PenaltyLimitBC

!syntax description /BCs/PenaltyLimitBC

## Description

`PenaltyLimitBC` is a `IntegratedBC` used for enforcing Dirichlet boundary conditions.
This implementation is very similar to [`PenaltyDirichletBC`](/PenaltyDirichletBC.md),
except that the applied boundary condition value is conditional if the variable value is greater than or less than the set value.

## Example Input Syntax

!listing test/tests/bcs/penalty_limit_bc/nonad_greaterthan.i block=BCs/penalty_limit

!listing test/tests/bcs/penalty_limit_bc/nonad_lessthan.i block=BCs/penalty_limit

!syntax parameters /BCs/PenaltyLimitBC

!syntax inputs /BCs/PenaltyLimitBC

!syntax children /BCs/PenaltyLimitBC

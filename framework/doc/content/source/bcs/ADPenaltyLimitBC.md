# ADPenaltyLimitBC

!syntax description /BCs/ADPenaltyLimitBC

## Description

`ADPenaltyLimitBC` is a `ADIntegratedBC` used for enforcing Dirichlet boundary conditions.
This implementation is very similar to [`ADPenaltyDirichletBC`](/ADPenaltyDirichletBC.md),
except that the applied boundary condition value is conditional if the variable value is greater than or less than the set value.

## Example Input Syntax

!listing test/tests/bcs/penalty_limit_bc/ad_greaterthan.i block=BCs/penalty_limit

!listing test/tests/bcs/penalty_limit_bc/ad_lessthan.i block=BCs/penalty_limit

!syntax parameters /BCs/PenaltyLimitBC

!syntax inputs /BCs/PenaltyLimitBC

!syntax children /BCs/PenaltyLimitBC

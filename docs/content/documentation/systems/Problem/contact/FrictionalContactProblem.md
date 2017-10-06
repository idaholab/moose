# FrictionalContactProblem
!syntax description /Problem/FrictionalContactProblem

## Description

`FrictionalContactProblem` is
used when a user wants to use kinematic (default) enforcement of frictional contact. If a user
wants to use the penalty method for frictional contact the `friction_coefficient` needs to be
specified in the `[Contact]` block and the `model` parameter set to `coulomb`.

It can be seen that a signifcant amount of auxiliary variables are required to be added to the
input file to make `FrictionalContactProblem` work. In addition references to saved variables
as in the `ReferenceResidualProblem` case is also required. If you would like to use
`FrictionalContactProblem` please contact a developer for assistance.

!syntax parameters /Problem/FrictionalContactProblem

!syntax inputs /Problem/FrictionalContactProblem

!syntax children /Problem/FrictionalContactProblem

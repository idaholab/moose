# Penalty Dirichlet Old Value Nodal Kernel

!syntax description /NodalKernels/PenaltyDirichletOldValuePD

## Description

The `PenaltyDirichletOldValuePD` NodalKernel is used to apply Dirichlet boundary conditions with value of old variable solution using the penalty method for nodesets. It is similar to imposing Dirichlet boundary condition using penalty method, but implemented using nodal kernel.

## Example Syntax

!listing test/tests/nodalkernels/penalty_dirichlet_old_value.i

!syntax parameters /NodalKernels/PenaltyDirichletOldValuePD

!syntax inputs /NodalKernels/PenaltyDirichletOldValuePD

!syntax children /NodalKernels/PenaltyDirichletOldValuePD

# ADRobinBC

!syntax description /BCs/ADRobinBC

## Description

`ADRobinBC` imposes a Robin boundary condition on a surface. `ADRobinBC` is
an integrated boundary condition that allows for the combination of
Dirichlet and Neumann boundary conditions.

Note that `ADRobinBC` computes its Jacobian using automatic differentiation.

## Example Input Syntax

!listing test/tests/bcs/ad_bcs/ad_bc.i block=BCs

!syntax parameters /BCs/ADRobinBC

!syntax inputs /BCs/ADRobinBC

!syntax children /BCs/ADRobinBC

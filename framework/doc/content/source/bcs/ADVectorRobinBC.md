# ADVectorRobinBC

!syntax description /BCs/ADVectorRobinBC

## Description

`ADVectorRobinBC` imposes a Robin boundary condition on a boundary.
`ADVectorRobinBC` is
a vector integrated boundary condition that allows for the combination of
Dirichlet and Neumann boundary conditions.

Note that `ADVectorRobinBC` computes its Jacobian using automatic differentiation.

## Example Input Syntax

!listing test/tests/bcs/ad_bcs/vector_ad_bc.i block=BCs

!syntax parameters /BCs/ADVectorRobinBC

!syntax inputs /BCs/ADVectorRobinBC

!syntax children /BCs/ADVectorRobinBC

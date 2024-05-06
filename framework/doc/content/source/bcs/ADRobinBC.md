# ADRobinBC

!syntax description /BCs/ADRobinBC

## Description

`ADRobinBC` imposes a Robin boundary condition on a boundary. `ADRobinBC` is
an integrated boundary condition similar to a [NeumannBC.md], but the derivative is
set to a function of the variable instead of a constant.

Note that `ADRobinBC` computes its Jacobian using automatic differentiation.

## Example Input Syntax

!listing test/tests/bcs/ad_bcs/ad_bc.i block=BCs

!syntax parameters /BCs/ADRobinBC

!syntax inputs /BCs/ADRobinBC

!syntax children /BCs/ADRobinBC

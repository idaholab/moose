# RobinBC

!syntax description /BCs/RobinBC

## Description

`RobinBC` imposes a Robin boundary condition on a boundary. `RobinBC` is
an integrated boundary condition similar to a [NeumannBC.md], but the derivative is
set to a function of the variable instead of a constant.

Note that `RobinBC` computes its Jacobian using automatic differentiation.

## Example Input Syntax

!listing test/tests/bcs/ad_bcs/ad_bc.i block=BCs

!syntax parameters /BCs/RobinBC

!syntax inputs /BCs/RobinBC

!syntax children /BCs/RobinBC

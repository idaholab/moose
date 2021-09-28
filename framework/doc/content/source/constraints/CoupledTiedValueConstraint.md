# CoupledTiedValueConstraint

!syntax description /Constraints/CoupledTiedValueConstraint

The constraint is imposed strongly on both sides of the interface, contributing to the residual.

## Example input syntax

In this example, the value of variable `u` and `v`, defined in the left and right domains,
is tied between the left and right boundaries of the two disconnected domains using a
`CoupledTiedValueConstraint`.

!listing test/tests/constraints/coupled_tied_value_constraint/coupled_tied_value_constraint.i block=Constraints

!syntax parameters /Constraints/CoupledTiedValueConstraint

!syntax inputs /Constraints/CoupledTiedValueConstraint

!syntax children /Constraints/CoupledTiedValueConstraint

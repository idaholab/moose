# TiedValueConstraint

!syntax description /Constraints/TiedValueConstraint

The constraint is imposed strongly on both sides of the interface, contributing to the residual.
Please see the [CoupledTiedValueConstraint.md] for the two variables case.
The `TiedValueConstraint` may also be used with two different variables, however the off diagonal terms
in the Jacobian for the constraint are only implemented in the [CoupledTiedValueConstraint.md].

## Example input syntax

In this example, the value of variable `u` defined in the left and right domains,
is tied between the left and right boundaries of the two disconnected domains using a
`TiedValueConstraint`.

!listing test/tests/constraints/tied_value_constraint/tied_value_constraint_test.i block=Constraints

!syntax parameters /Constraints/TiedValueConstraint

!syntax inputs /Constraints/TiedValueConstraint

!syntax children /Constraints/TiedValueConstraint

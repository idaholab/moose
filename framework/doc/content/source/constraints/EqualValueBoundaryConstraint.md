# EqualValueBoundaryConstraint

!syntax description /Constraints/EqualValueBoundaryConstraint

The difference between the two variables (or a single variable) values on the primary node and the
secondary node (or boundary) is simply multiplied by a penalty factor then added to the
residual.

## Example input syntax

In this example, the variable `diffused` is constrained to be equal on node `45` and the `top` boundary, using
a 10e6 penalty factor.

!listing test/tests/constraints/equal_value_boundary_constraint/equal_value_boundary_constraint_test.i block=Constraints

!syntax parameters /Constraints/EqualValueBoundaryConstraint

!syntax inputs /Constraints/EqualValueBoundaryConstraint

!syntax children /Constraints/EqualValueBoundaryConstraint

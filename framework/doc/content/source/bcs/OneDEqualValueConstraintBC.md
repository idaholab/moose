# OneDEqualValueConstraintBC

!syntax description /BCs/OneDEqualValueConstraintBC

The constraint is imposed using a Lagrange multiplier, a scalar variable.

More information about the mortar method may be found in the
[`Constraints documentation`](syntax/Constraints/index.md).

## Example input syntax

In this example, two 1D lines are connected using the `OneDEqualValueConstraintBC` as a mortar constraint.
The sign to impose on the variable value on the boundary nodes is specified with the [!param](/BCs/OneDEqualValueConstraintBC/vg) parameter.
The Lagrange multiplier, `lm`, is also subjected to a `NodalEqualValueConstraint` to achieve the continuity
at the interface.

!listing test/tests/mortar/1d/1d.i block=BCs

!syntax parameters /BCs/OneDEqualValueConstraintBC

!syntax inputs /BCs/OneDEqualValueConstraintBC

!syntax children /BCs/OneDEqualValueConstraintBC

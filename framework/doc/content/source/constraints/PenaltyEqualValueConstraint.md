# PenaltyEqualValueConstraint

The `PenaltyEqualValueConstraint` class is used to enforce continuity of a
variable across a mortar interface or in a periodic boundary
condition. The variable is specified using the `primary_variable`
parameter. If the solution values to be matched are between different variables, the
`secondary_variable` parameter can also be supplied. The enforcement takes place in a penalty sense, 
which eliminates the need to supply Lagrange multipliers.

See [EqualValueConstraint](/EqualValueConstraint.md) for exact enforcement using Lagrange multipliers.

!listing test/tests/mortar/continuity-3d-non-conforming/continuity_penalty_sphere_hex8.i block=Constraints

!syntax description /Constraints/PenaltyEqualValueConstraint

!syntax parameters /Constraints/PenaltyEqualValueConstraint

!syntax inputs /Constraints/PenaltyEqualValueConstraint

!syntax children /Constraints/PenaltyEqualValueConstraint

!bibtex bibliography

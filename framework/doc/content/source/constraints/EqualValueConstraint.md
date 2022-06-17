# EqualValueConstraint

The `EqualValueConstraint` class is used to enforce continuity of a
variable across a mortar interface or in a periodic boundary
condition. The variable is specified using the `primary_variable`
parameter. If the solution values to be matched are between different variables, the
`secondary_variable` parameter can also be supplied. Lagrange multipliers are used
to perform the constraint enforcement.

See [PenaltyEqualValueConstraint](/PenaltyEqualValueConstraint.md) for penalty-based enforcement of analogous constraints.

!syntax description /Constraints/EqualValueConstraint

!syntax parameters /Constraints/EqualValueConstraint

!syntax inputs /Constraints/EqualValueConstraint

!syntax children /Constraints/EqualValueConstraint

!bibtex bibliography

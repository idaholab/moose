# EqualGradientConstraint

The `EqualGradientConstraint` class is used to enforce continuity of a component
of a variable gradient across a mortar interface or in a periodic boundary
condition. The gradient component to be matched is specified using the unsigned
parameter `component`. The variable is specified using the `primary_variable`
parameter. If the gradients to be matched are between different variables, the
`secondary_variable` parameter can also be supplied. Lagrange multipliers are used
to perform the constraint enforcement.

!syntax description /Constraints/EqualGradientConstraint

!syntax parameters /Constraints/EqualGradientConstraint

!syntax inputs /Constraints/EqualGradientConstraint

!syntax children /Constraints/EqualGradientConstraint

!bibtex bibliography

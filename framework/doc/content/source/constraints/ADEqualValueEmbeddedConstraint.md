# ADEqualValueEmbeddedConstraint

!syntax description /Constraints/ADEqualValueEmbeddedConstraint

This version of `EqualValueEmbeddedConstraint` uses automatic differentiation, see [EqualValueEmbeddedConstraint.md] for information on formulation.

!alert warning title=`ADEqualValueEmbeddedConstraint` only works for PENALTY formulation
Automatic differentation cannot be used to compute Jacobian terms for [!param](/Constraints/ADEqualValueEmbeddedConstraint/formulation) = KINEMATIC because the KINEMATIC implementation enforces the constraint by enforcing the residual value on the secondary node prior to application of the constraint, $r_{s,copy}$ in [!eqref](EqualValueEmbeddedConstraint.md#eqn:kinematic).  This secondary node residual value is a nonAD copy of the residual and cannot be used to compute derivatives for the Jacobian.

!syntax parameters /Constraints/ADEqualValueEmbeddedConstraint

!syntax inputs /Constraints/ADEqualValueEmbeddedConstraint

!syntax children /Constraints/ADEqualValueEmbeddedConstraint

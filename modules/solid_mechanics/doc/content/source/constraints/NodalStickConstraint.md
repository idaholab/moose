# NodalStickConstraint

!syntax description /Constraints/NodalStickConstraint

Both kinematic and penalty formulations are implemented.

For the penalty formulation, the contribution to the residual for the constrained nodes on the primary side is equal to

!equation
\text{penalty} (u_{primary} - u_{secondary})

and the opposite on the secondary side, where $u$ is usually the displacement component variable value at the node for
mechanical contact applications.

!alert note
The primary and secondary variables must be different for this implementation of the nodal
stick constraint.

!syntax parameters /Constraints/NodalStickConstraint

!syntax inputs /Constraints/NodalStickConstraint

!syntax children /Constraints/NodalStickConstraint
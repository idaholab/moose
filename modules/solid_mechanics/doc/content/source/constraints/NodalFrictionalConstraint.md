# NodalFrictionalConstraint

!syntax description /Constraints/NodalFrictionalConstraint

Only a penalty formulation is implemented.
First, the previous time step tangential force is computed as:

!equation
F_{old} = \text{tangeantial penalty} (u_{primary-old} - u_{secondary-old})

If it is greater than the allowed frictional force (normal force times the friction coefficient),
then the local nodes are in the slippage regime and the old force is recomputed as:

!equation
F_{old} = \text{friction coefficient} * \text{normal force} * \dfrac{F_{old}}{|F_{old}|}

From the previous time step force $F_{old}$, and the previous and current values of $u$ on the
primary and secondary side, the current force is computed as:

!equation
F = F_{old} + ((u_{secondary-old} - u_{secondary}) - (u_{primary} - u_{primary-old})) *
          \text{tangeantial penalty}

If it is greater than the allowed frictional force (normal force times the friction coefficient),
then the local nodes are in the slippage regime and the current force is recomputed as:

!equation
F = \text{friction coefficient} * \text{normal force} * \dfrac{F}{|F|}

and the opposite on the secondary side, where $u$ is usually the displacement component variable value for
mechanical contact applications.

!alert note
The primary and secondary variables must be different for this implementation of the nodal
frictional constraint.

!syntax parameters /Constraints/NodalFrictionalConstraint

!syntax inputs /Constraints/NodalFrictionalConstraint

!syntax children /Constraints/NodalFrictionalConstraint
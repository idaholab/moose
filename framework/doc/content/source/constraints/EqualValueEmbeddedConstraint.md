# EqualValueEmbeddedConstraint

!syntax description /Constraints/EqualValueEmbeddedConstraint

This is a constraint acting upon overlapping portions of two blocks, a secondary block and a primary block. The constraint enforces the secondary variable on the secondary block and the primary variable on the primary block to have the same values. The mesh dimensions of the two blocks do not have to match.

The constraint iterates through all the nodes on the secondary block and searches for a primary element that contains each secondary node. If a secondary node is located within an element, then a constraint is applied to force the secondary node to have the same value as the solution variable in the primary element, evaluated at the location of the secondary point.

This can be used for a number of applications. For example, in mechanics problems, it can be used to connect lower dimensional elements such as 1D truss elements to 2D or 3D continuum elements. This can be used to model reinforcement in a way that does not require the reinforcement and continuum meshes to have coincident nodes.

## Mathematical Formulation

Options are available to control how this constraint is applied:

### Kinematic

This option strictly enforces value of the solution at the secondary nodes to be equal to the value in the primary element at that point. The constraint is enforced by updating the secondary residual $r_s$ and primary residual $r_m$ as:
\begin{equation}
\begin{aligned}
r_s &= r_s + f_c + k_p(u_{secondary} - u_{primary})\\
r_m &= r_m + \phi_i r_{s,copy}
\end{aligned}
\end{equation}
where $r_{s,copy}$ is a copy of the residual of the secondary node before the constraint is applied and $k_p$ is the user-specified penalty parameter. This formulation uses the penalty parameter only to penalize the error, and the converged solution has no error due the penalty. The penalty factor must be specified, and should be consistent with the scaling for the solution variable to which this is applied.

### Penalty

This option uses a penalty formulation in which the error in the solution is proportional to a user-specified penalty parameter $k_p$. The constraint is enforced by modifying the secondary and primary residual
The constraint is enforced by updating the secondary residual $r_s$ and primary residual $r_m$ as:
\begin{equation}
\begin{aligned}
r_s &= r_s + k_p(u_{secondary} - u_{primary})\\
r_m &= r_m - \phi_i k_p(u_{secondary} - u_{primary})
\end{aligned}
\end{equation}
where $r_{s,copy}$ is the ghosted residual. The penalty parameter must be selected carefully, as small values lead to large differences between the secondary node's solution and the solution in the primary element, while large values may lead to poor convergence.

!syntax parameters /Constraints/EqualValueEmbeddedConstraint

!syntax inputs /Constraints/EqualValueEmbeddedConstraint

!syntax children /Constraints/EqualValueEmbeddedConstraint

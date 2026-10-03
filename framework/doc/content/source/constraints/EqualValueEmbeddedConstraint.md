# EqualValueEmbeddedConstraint / ADEqualValueEmbeddedConstraint

!syntax description /Constraints/EqualValueEmbeddedConstraint

This is a constraint acting upon overlapping portions of two blocks, a secondary block and a primary block. The constraint enforces the secondary variable on the secondary block and the primary variable on the primary block to have the same values. The mesh dimensions of the two blocks do not have to match.

The constraint iterates through all the nodes on the secondary block and searches for a primary element that contains each secondary node. If a secondary node is located within an element, then a constraint is applied to force the secondary node to have the same value as the solution variable in the primary element, evaluated at the location of the secondary point.

This can be used for a number of applications. For example, in mechanics problems, it can be used to connect lower dimensional elements such as 1D truss elements to 2D or 3D continuum elements. This can be used to model reinforcement in a way that does not require the reinforcement and continuum meshes to have coincident nodes.

## Mathematical Formulation

Options are available to control how this constraint is applied:

### Kinematic

This option strictly enforces value of the solution at the secondary nodes to be equal to the value in the primary element at that point. The constraint is enforced by updating the secondary residual $r_s$ and primary residual $r_m$ as:
\begin{equation}
\label{eqn:kinematic}
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

### Rows

This option assembles no residual and no Jacobian. For each secondary node it hands libMesh one degree of freedom constraint row that makes the secondary value the shape-function interpolation of the primary variable at that node,
\begin{equation}
u_{secondary} = \sum_i N_i(\xi_s)\, u_i ,
\end{equation}
where $u_i$ are the degrees of freedom of [!param](/Constraints/EqualValueEmbeddedConstraint/primary_variable) on the primary element that contains the node, $\xi_s$ is the reference coordinate of that node in that element, and $N_i$ are the shape functions of the primary variable there. The reference coordinate comes from `FEMap::inverse_map` and the weights from `FEInterface::shape` on the undisplaced mesh, which is the same point and the same interpolation the two residual formulations use. libMesh condenses the constrained degrees of freedom out of every matrix and vector MOOSE assembles, so the tie is exact, no penalty parameter enters, and [!param](/Constraints/EqualValueEmbeddedConstraint/penalty) may be omitted. [NodalConstraint.md#rows] describes the mechanism and its parallel rules in more detail.

A node that both blocks share constrains its own degree of freedom. The tie already holds there, so no row is added for it. A secondary node that an enabled nodal boundary condition already pins is skipped as well, so that the boundary condition keeps the node, as it does under the other two formulations.

!alert note title=The secondary-to-primary pairing is not rebuilt on mesh change
Every formulation of this constraint locates the primary element of each secondary node once, when the object is constructed. Mesh adaptation is therefore not supported with any of the three, including `rows`.

!alert warning title=`ADEqualValueEmbeddedConstraint` does not support the KINEMATIC formulation
Automatic differentiation cannot be used to compute Jacobian terms for [!param](/Constraints/ADEqualValueEmbeddedConstraint/formulation) = KINEMATIC because the KINEMATIC implementation enforces the constraint by enforcing the residual value on the secondary node prior to application of the constraint, $r_{s,copy}$ in [!eqref](eqn:kinematic).  This secondary node residual value is a nonAD copy of the residual and cannot be used to compute derivatives for the Jacobian. The PENALTY and ROWS formulations carry no such restriction, and ROWS computes no Jacobian at all.

!syntax parameters /Constraints/EqualValueEmbeddedConstraint

!syntax inputs /Constraints/EqualValueEmbeddedConstraint

!syntax children /Constraints/EqualValueEmbeddedConstraint

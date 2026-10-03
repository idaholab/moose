# EqualValueBoundaryConstraint

!syntax description /Constraints/EqualValueBoundaryConstraint

`EqualValueBoundaryConstraint` ties every secondary node to a single primary node with unit weight,
so that the variable takes the same value at all of them,

\begin{equation}
u_s = u_p .
\end{equation}

The secondary nodes are the nodes of the boundary named in
[!param](/Constraints/EqualValueBoundaryConstraint/secondary), or the node ids listed in
[!param](/Constraints/EqualValueBoundaryConstraint/secondary_node_ids). The primary node is the one
named in [!param](/Constraints/EqualValueBoundaryConstraint/primary), the one located by
[!param](/Constraints/EqualValueBoundaryConstraint/primary_node_coord), or, when neither is given,
the first node of the secondary set. The primary node itself is skipped when it is also in the
secondary set.

[!param](/Constraints/EqualValueBoundaryConstraint/formulation) selects how the relation is
enforced, and [NodalConstraint.md] describes what the three values do. With `penalty`, which is the
default, the difference between the value on the primary node and the value on each secondary node
is multiplied by [!param](/Constraints/EqualValueBoundaryConstraint/penalty) and added to the
residual, so the tie holds only to the inverse of that factor. `kinematic` moves the secondary
node's residual onto the primary node and also needs the penalty factor. `rows` assembles no
residual: it hands one degree of freedom constraint row per secondary node to the `DofMap`, which
holds the tie exactly and needs no penalty factor, so
[!param](/Constraints/EqualValueBoundaryConstraint/penalty) may be omitted with it.

Under `rows` the nodes of [!param](/Constraints/EqualValueBoundaryConstraint/secondary) are read at
the moment the rows are built rather than at construction. libMesh rebuilds its constraints during
mesh adaptation, before MOOSE delivers the mesh-changed notification, so the nodes that refinement
adds to that boundary are tied in the same rebuild that created them and the boundary never carries
an untied node.

## Example input syntax

In this example, the variable `diffused` is constrained to be equal on every node of the `top`
boundary, using a 10e6 penalty factor.

!listing test/tests/constraints/equal_value_boundary_constraint/equal_value_boundary_constraint_test.i block=Constraints

!syntax parameters /Constraints/EqualValueBoundaryConstraint

!syntax inputs /Constraints/EqualValueBoundaryConstraint

!syntax children /Constraints/EqualValueBoundaryConstraint

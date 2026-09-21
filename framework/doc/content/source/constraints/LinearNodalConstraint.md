# LinearNodalConstraint

!syntax description /Constraints/LinearNodalConstraint

`LinearNodalConstraint` makes the value of a variable at each secondary node a fixed linear
combination of its values at a list of primary nodes,

\begin{equation}
u_s = \sum_{j} w_j\, u_{p_j} ,
\end{equation}

with the primary node ids in [!param](/Constraints/LinearNodalConstraint/primary) and the weights
$w_j$ in [!param](/Constraints/LinearNodalConstraint/weights), which must be of the same length.
The secondary nodes come either from
[!param](/Constraints/LinearNodalConstraint/secondary_node_ids) or from
[!param](/Constraints/LinearNodalConstraint/secondary_node_set); supply one of the two. Every
secondary node receives the same relation.

[!param](/Constraints/LinearNodalConstraint/formulation) selects how the relation is enforced, and
[NodalConstraint.md] describes what the three values do. In short, `penalty` and `kinematic`
assemble a residual and need [!param](/Constraints/LinearNodalConstraint/penalty), while `rows`
hands one degree of freedom constraint row per secondary node to the `DofMap`, enforces the tie
exactly, needs no penalty parameter, and reaches the eigen matrices of an
[Eigenvalue executioner](EigenProblem.md) that a penalty tie never reaches.

Two behaviors are specific to `rows` on this object. The nodes of
[!param](/Constraints/LinearNodalConstraint/secondary_node_set) are read when the rows are built, so
a node that mesh refinement adds to that node set is tied in the same rebuild that created it. And
[!param](/Constraints/LinearNodalConstraint/weights) is controllable: a
[Controls](syntax/Controls/index.md) object that changes it between solves makes the framework
rebuild the rows with the new weights. Under `penalty` and `kinematic` the object keeps the weights
it read at construction.

## Example input syntax

In this example the variable `u` at node 4 is tied to ten times its value at node 0. The input does
not set [!param](/Constraints/LinearNodalConstraint/formulation), so the tie is enforced with the
default penalty; the test that owns this input runs it once per formulation and records the tie
error $u_s - 10\,u_p$ that each one leaves.

!listing test/tests/constraints/nodal_constraint/linear_nodal_constraint_tie_error.i block=Constraints

A tie enforced with constraint rows needs no penalty parameter. This input ties node 4 to node 0
with a unit weight and ramps that weight between time steps with a `Control`:

!listing test/tests/constraints/nodal_constraint/rows_reinit.i block=Constraints

!syntax parameters /Constraints/LinearNodalConstraint

!syntax inputs /Constraints/LinearNodalConstraint

!syntax children /Constraints/LinearNodalConstraint

!bibtex bibliography

# RBE2Constraint

!syntax description /Constraints/RBE2Constraint

## Description

`RBE2Constraint` is the MOOSE equivalent of the Nastran RBE2 rigid element. One independent node
carries three translations and three rotations, and every node of a dependent boundary follows it
as if a rigid body joined them. The constraint is kinematic: it adds
[MultiPointConstraint.md] rows to the `DofMap`, so it adds no stiffness, no mass and no penalty
parameter of its own.

Use it to attach a face, an edge or a set of nodes to a single node that carries the six degrees of
freedom of a bolt head, a bearing, a lumped mass, or a beam end.

### Kinematics

Let $\mathbf{x}_0$ be the position of the independent node, $\mathbf{u}_0$ its translation and
$\boldsymbol{\theta}_0$ its rotation. For a dependent node $k$ at $\mathbf{x}_k$, write the lever
arm $\mathbf{r}_k = \mathbf{x}_k - \mathbf{x}_0$, taken once from the undisplaced mesh. Small
rotations give

\begin{equation}
\mathbf{u}_k = \mathbf{u}_0 + \boldsymbol{\theta}_0 \times \mathbf{r}_k
             = \mathbf{u}_0 - [\mathbf{r}_k]_\times \boldsymbol{\theta}_0 ,
\end{equation}

where $[\mathbf{a}]_\times$ is the skew matrix with
$[\mathbf{a}]_\times \mathbf{b} = \mathbf{a} \times \mathbf{b}$. Written out, the row of the $x$
translation of node $k$ holds three coefficients, all of them on the independent node,

\begin{equation}
u_{k,x} = u_{0,x} + r_{k,z}\,\theta_{0,y} - r_{k,y}\,\theta_{0,z},
\end{equation}

and the $y$ and $z$ rows follow by cyclic permutation. The translations come from
[!param](/Constraints/RBE2Constraint/displacements) and the rotations from
[!param](/Constraints/RBE2Constraint/rotations).

When [!param](/Constraints/RBE2Constraint/dependent_dofs) is `translations_and_rotations`, each
dependent node also receives three rotation rows,

\begin{equation}
\boldsymbol{\theta}_k = \boldsymbol{\theta}_0 ,
\end{equation}

which is the small-rotation statement that the two nodes share an orientation. Use this setting
when the dependent nodes carry rotations, as beam and shell nodes do. Leave the default,
`translations`, for continuum nodes, which solve translations only.

A dependent node at the position of the independent node has $\mathbf{r}_k = \mathbf{0}$, and its
rows reduce to $\mathbf{u}_k = \mathbf{u}_0$. That is the splice of two coincident but distinct
nodes.

### Operating envelope

The coefficients are constant, built once from the undisplaced mesh, so the constraint describes
small motions about the reference configuration. It is not a large-rotation rigid body, and
`use_displaced_mesh = true` raises an error.

The object is three-dimensional. [!param](/Constraints/RBE2Constraint/displacements) and
[!param](/Constraints/RBE2Constraint/rotations) each take exactly three variables, and any other
count is an error.

[!param](/Constraints/RBE2Constraint/independent_boundary) must resolve to exactly one node, and
that node must solve all six variables. A continuum block does not solve rotations, so place the
independent node on a block that does, such as a beam block or a stub element. The constraint
reports an error naming the missing variable when a rotation has no degree of freedom there.

Errors also arise when a node of [!param](/Constraints/RBE2Constraint/dependent_boundary) is the
independent node itself, when a dependent node does not solve one of the three displacements, and
when [!param](/Constraints/RBE2Constraint/dependent_dofs) is `translations_and_rotations` but a
dependent node carries no rotation degree of freedom.

[!param](/Constraints/RBE2Constraint/dependent_boundary) accepts a node set or a side set, because
MOOSE builds a node list for both.

Because the rows are rebuilt whenever libMesh rebuilds the constraints of the system, the nodes
that mesh refinement creates on the dependent boundary receive their rows automatically.

### Verification

The regression tests measure three properties of the rows. A prescribed rotation of the independent
node reproduces the exact rigid field over the whole dependent body, to a largest nodal L2 error of
9.2e-14. Two beam segments spliced with `dependent_dofs = translations_and_rotations` deflect as one
continuous cantilever, with a tip deflection of 7.2262e-3 against $PL^{3}/3EI = 7.2265e-3$, a
relative agreement of 3.3e-5 that the Timoshenko shear term of 5.9e-6 and the discretization error
account for. A tip mass carried on the independent node of a spider gives a first bending
eigenvalue of 999.27, within 0.5 percent of the 994.31 of the same cantilever with the mass lumped
on the face nodes.

## Example input syntax

The spider ties the free face of a cantilever, `tie_face`, to the first node of a short stub that
carries the six degrees of freedom. The dependent nodes are continuum nodes, so
[!param](/Constraints/RBE2Constraint/dependent_dofs) stays on `translations`, while
[!param](/Constraints/RBE2Constraint/rotations) names the rotation variables of the independent
node.

!listing modules/solid_mechanics/test/tests/multipoint_constraints/rbe2/rbe2_tip_mass_eigen.i block=Constraints

That input runs under [EigenProblem.md]. A constrained `DofMap` reaches the eigen matrices only
through the nonlinear eigen solve, so the executioner sets `solve_type = NEWTON` and
`n_eigen_pairs = 1`.
[The eigen tag section of the Constraints syntax page](syntax/Constraints/index.md#eigen-tag) explains why.

!syntax parameters /Constraints/RBE2Constraint

!syntax inputs /Constraints/RBE2Constraint

!syntax children /Constraints/RBE2Constraint

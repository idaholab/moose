# RBE2Constraint

!syntax description /Constraints/RBE2Constraint

## Description

`RBE2Constraint` is a rigid element. One independent node has three translations and three
rotations. Each node of a dependent boundary moves with the independent node, as if a rigid body
connects them. The constraint adds [MultiPointConstraint.md] exact constraints to the `DofMap`. It
adds no stiffness, no mass, and no penalty parameter.

Use it to attach a face, an edge, or a set of nodes to one node with six degrees of freedom. This
node can represent a bolt head, a bearing, a lumped mass, or a beam end.

### Kinematics

Let $\mathbf{x}_0$ be the position of the independent node, $\mathbf{u}_0$ its translation and
$\boldsymbol{\theta}_0$ its rotation. For a dependent node $k$ at $\mathbf{x}_k$, the lever arm is
$\mathbf{r}_k = \mathbf{x}_k - \mathbf{x}_0$ on the undisplaced mesh. For small rotations,

\begin{equation}
\mathbf{u}_k = \mathbf{u}_0 + \boldsymbol{\theta}_0 \times \mathbf{r}_k
             = \mathbf{u}_0 - [\mathbf{r}_k]_\times \boldsymbol{\theta}_0 ,
\end{equation}

where $[\mathbf{a}]_\times$ is the skew matrix with
$[\mathbf{a}]_\times \mathbf{b} = \mathbf{a} \times \mathbf{b}$. The exact constraint of the $x$
translation of node $k$ has three coefficients, all on the independent node:

\begin{equation}
u_{k,x} = u_{0,x} + r_{k,z}\,\theta_{0,y} - r_{k,y}\,\theta_{0,z} .
\end{equation}

A cyclic permutation gives the $y$ and $z$ exact constraints. The translations are the variables
in [!param](/Constraints/RBE2Constraint/displacements). The rotations are the variables in
[!param](/Constraints/RBE2Constraint/rotations).

When [!param](/Constraints/RBE2Constraint/dependent_dofs) is `translations_and_rotations`, each
dependent node also gets three rotation exact constraints:

\begin{equation}
\boldsymbol{\theta}_k = \boldsymbol{\theta}_0 .
\end{equation}

For small rotations, this means that the two nodes have the same orientation. Use this setting
when the dependent nodes have rotations, for example beam and shell nodes. Keep the default,
`translations`, for continuum nodes, which have translations only.

A dependent node at the position of the independent node has $\mathbf{r}_k = \mathbf{0}$. Its
exact constraints become $\mathbf{u}_k = \mathbf{u}_0$. This splices two different nodes at the
same position.

### Operating envelope

The coefficients are constant, and the object computes them from the undisplaced mesh, so the
constraint describes small motions about the reference configuration. It is not a large-rotation
rigid body. `use_displaced_mesh = true` raises an error.

The object is three-dimensional. [!param](/Constraints/RBE2Constraint/displacements) and
[!param](/Constraints/RBE2Constraint/rotations) each take exactly three variables. A different
number raises an error.

[!param](/Constraints/RBE2Constraint/independent_boundary) must contain exactly one node. This
node must have a degree of freedom for all six variables. A continuum block has no rotations, so put
the independent node on a block that has them, for example a beam block or a stub element. When a
variable has no degree of freedom at the independent node, the error gives the name of the
variable.

The constraint also raises an error in these conditions:

- A node of [!param](/Constraints/RBE2Constraint/dependent_boundary) is the independent node.
- A dependent node has no degree of freedom for one of the three displacements.
- [!param](/Constraints/RBE2Constraint/dependent_dofs) is `translations_and_rotations`, and a
  dependent node has no rotation degree of freedom.

[!param](/Constraints/RBE2Constraint/dependent_boundary) accepts a node set or a side set, because
MOOSE makes a node list for both.

libMesh calls `ExactConstraintFunctor` each time it rebuilds the constraints of the system. The
new nodes that mesh refinement adds to the dependent boundary then get their exact constraints.

### Verification

The regression tests measure three properties of the exact constraints:

- A prescribed rotation of the independent node gives the exact rigid field on all of the dependent
  body. The largest nodal L2 error is 9.2e-14.
- Two beam segments spliced with `dependent_dofs = translations_and_rotations` deflect as one
  cantilever. The tip deflection is 7.2262e-3, and $PL^{3}/3EI = 7.2265e-3$. The relative
  difference of 3.3e-5 comes from the Timoshenko shear term of 5.9e-6 and the discretization error.
- A tip mass on the independent node of a spider gives a first bending eigenvalue of 999.27. The
  same cantilever with the mass lumped on the face nodes gives 994.31, a difference below 0.5
  percent.

## Example input syntax

The spider ties the free face of a cantilever, `tie_face`, to the first node of a short stub. This
node has the six degrees of freedom. The dependent nodes are continuum nodes, so
[!param](/Constraints/RBE2Constraint/dependent_dofs) keeps the value `translations`.
[!param](/Constraints/RBE2Constraint/rotations) gives the rotation variables of the independent
node.

!listing modules/solid_mechanics/test/tests/multipoint_constraints/rbe2/rbe2_tip_mass_eigen.i block=Constraints

This input uses [EigenProblem.md]. Exact constraints in an eigenvalue problem need a nonlinear
eigen solve, so the executioner sets `solve_type = NEWTON` and `n_eigen_pairs = 1`.
[The eigen tag section of the Constraints syntax page](syntax/Constraints/index.md#eigen-tag) gives
more information.

!syntax parameters /Constraints/RBE2Constraint

!syntax inputs /Constraints/RBE2Constraint

!syntax children /Constraints/RBE2Constraint

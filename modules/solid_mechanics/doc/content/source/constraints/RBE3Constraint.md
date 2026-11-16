# RBE3Constraint

!syntax description /Constraints/RBE3Constraint

## Description

`RBE3Constraint` is an interpolation element. One reference node depends on a set of independent
nodes. The reference node moves as the weighted least-squares rigid-body fit of their translations.
The transpose of this relation distributes a load on the reference node to the independent nodes.

The fit uses only the translations of the independent nodes. It does not use their rotations.

The constraint adds [MultiPointConstraint.md] exact constraints to the `DofMap`, so it adds no
stiffness. Use it to apply a load or a mass at a point. Unlike [RBE2Constraint.md], it does not make
the region below the point rigid.

### Least-squares fit

Node $i$ of the independent set has the weight $w_i$, the position $\mathbf{x}_i$, and the
translation $\mathbf{u}_i$. The total weight is $W = \sum_i w_i$, and the weighted centroid is
$\mathbf{x}_c = \sum_i w_i \mathbf{x}_i / W$. Let $\mathbf{r}_i = \mathbf{x}_i - \mathbf{x}_c$.
The offset of the reference node from the centroid is
$\mathbf{d} = \mathbf{x}_{\mathrm{ref}} - \mathbf{x}_c$. All positions are on the undisplaced
mesh.

The fit is the rigid-body motion that minimizes the weighted residual
$\sum_i w_i \lvert \mathbf{u}_i - (\bar{\mathbf{u}} + \boldsymbol{\theta} \times \mathbf{r}_i)
\rvert^{2}$. Its solution is

\begin{equation}
\bar{\mathbf{u}} = \frac{1}{W}\sum_i w_i \mathbf{u}_i, \qquad
\boldsymbol{\theta} = J^{-1} \sum_i w_i\, \mathbf{r}_i \times \mathbf{u}_i, \qquad
J = \sum_i w_i \left( \lvert \mathbf{r}_i \rvert^{2} I
    - \mathbf{r}_i \mathbf{r}_i^{\top} \right),
\end{equation}

where $J$ is the weighted second-moment tensor of the independent set. The reference node gets
this motion at its own position:

\begin{equation}
\mathbf{u}_{\mathrm{ref}} = \bar{\mathbf{u}} + \boldsymbol{\theta} \times \mathbf{d}, \qquad
\boldsymbol{\theta}_{\mathrm{ref}} = \boldsymbol{\theta} .
\end{equation}

The object always adds the three translation exact constraints. It adds the rotation exact
constraints only when you set [!param](/Constraints/RBE3Constraint/rotations). The coefficients of
node $i$ are the $3\times3$ blocks

\begin{equation}
\frac{\partial \mathbf{u}_{\mathrm{ref}}}{\partial \mathbf{u}_i}
  = \frac{w_i}{W} I - w_i\, [\mathbf{d}]_\times J^{-1} [\mathbf{r}_i]_\times ,
\qquad
\frac{\partial \boldsymbol{\theta}_{\mathrm{ref}}}{\partial \mathbf{u}_i}
  = w_i\, J^{-1} [\mathbf{r}_i]_\times ,
\end{equation}

where $[\mathbf{a}]_\times$ is the skew matrix with
$[\mathbf{a}]_\times \mathbf{b} = \mathbf{a} \times \mathbf{b}$. The weights are relative. If
you multiply all weights by the same factor, the result does not change.

### Load distribution

libMesh reduces the load vector with $C^{\top}$. A force $\mathbf{F}$ on the reference node gives
this force on node $i$:

\begin{equation}
\mathbf{F}_i = \frac{w_i}{W}\,\mathbf{F}
  + w_i \left( J^{-1}(\mathbf{d} \times \mathbf{F}) \right) \times \mathbf{r}_i .
\end{equation}

The first term is the weighted part of the resultant. The second term is a couple that carries
the moment of $\mathbf{F}$ about the centroid. This term is zero when the reference node is at the
weighted centroid. The sum of the distributed forces is $\mathbf{F}$, and their net moment about
the reference node is zero.

The table gives the two cases that the regression tests measure. The patch is the unit square with
corners $(0,0)$, $(1,0)$, $(1,1)$, and $(0,1)$. The reference node is at the center $(0.5, 0.5)$,
and a unit force acts along $z$. The measured values are the truss reactions of
`rbe3_force_split_equal.i`. The `rbe3_force_split_weighted` test runs the same input with the
second set of weights. In both cases, the difference from the closed form is less than 1e-12.

| weights | $\mathbf{x}_c$ | $\mathbf{d}$ | $F_z$ closed form | $F_z$ measured |
|---|---|---|---|---|
| 1, 1, 1, 1 | $(0.5, 0.5)$ | $(0, 0)$ | $1/4$ at each corner | 0.24999999999942 at each corner |
| 2, 1, 1, 1 | $(0.4, 0.4)$ | $(0.1, 0.1)$ | $2/7,\; 3/14,\; 2/7,\; 3/14$ | 0.28571428571489, 0.21428571428617, 0.28571428571489, 0.21428571428617 |

In the weighted case, $W = 5$ and $J^{-1}(\mathbf{d} \times \mathbf{F}) = (1/14, -1/14, 0)$.
Without the $\mathbf{d}$ term, the forces are $0.4,\ 0.2,\ 0.2,\ 0.2$.

### Operating envelope

The constraint adds no element. libMesh moves the stiffness at the reference node to the
independent nodes through $C^{\top} K C$. A soft stub element at the reference node is then the
only source of extra stiffness. An RBE3 constraint on four interior nodes of a clamped plate changes
the lowest eigenvalue by 1.4e-5 (relative). This change is proportional to the Young's modulus of
the stub.

The object is three-dimensional. [!param](/Constraints/RBE3Constraint/displacements) takes exactly
three variables. [!param](/Constraints/RBE3Constraint/rotations) is optional, and it also takes
exactly three variables.

[!param](/Constraints/RBE3Constraint/reference_boundary) must contain exactly one node. This node
must have a degree of freedom for each variable that the constraint ties at it. A continuum block
has no rotations. When you set [!param](/Constraints/RBE3Constraint/rotations), put the reference
node on a block that has rotations, for example a beam block.

[!param](/Constraints/RBE3Constraint/weights) has one weight for each entry of
[!param](/Constraints/RBE3Constraint/independent_boundaries). The weight applies to all nodes of
that node set. To give a node its own weight, put it in its own node set. Each weight must be
positive. The two parameters must have the same size. A node must not be in more than one of the
node sets, because it would then have two weights.

The object uses the rotation of the fit, and inverts $J$, only in two cases. The first case is
when you set [!param](/Constraints/RBE3Constraint/rotations). The second case is when the reference
node is farther from the weighted centroid than `libMesh::TOLERANCE` times the radius of gyration
of the independent set. $J$ is singular when the independent nodes are collinear or when there is
only one independent node. If the object must invert a singular $J$, it raises an error. One
independent node at the position of the reference node is valid when you do not set
[!param](/Constraints/RBE3Constraint/rotations).

The constraint also raises an error in these conditions:

- The reference node is one of the independent nodes.
- An independent node has no degree of freedom for one of the three displacements.
- The node sets contain no nodes.

The object computes the coefficients from the undisplaced mesh, so the constraint describes small
motions about the reference configuration. `use_displaced_mesh = true` raises an error.

## Example input syntax

This input ties the reference node at the center of a four-truss patch to the four corner nodes,
with equal weights. The load acts on the stub at the reference node. The constraint distributes the
load to the corners.

!listing modules/solid_mechanics/test/tests/multipoint_constraints/rbe3/rbe3_force_split_equal.i block=Constraints

!syntax parameters /Constraints/RBE3Constraint

!syntax inputs /Constraints/RBE3Constraint

!syntax children /Constraints/RBE3Constraint

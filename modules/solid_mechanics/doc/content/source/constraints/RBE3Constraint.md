# RBE3Constraint

!syntax description /Constraints/RBE3Constraint

## Description

`RBE3Constraint` is the MOOSE equivalent of the Nastran RBE3 interpolation element. A single
reference node is made dependent on a set of independent nodes: it moves as the weighted
least-squares rigid-body fit of their translations. By the transpose of that same relation, a load
applied at the reference node is distributed over the independent nodes.

Only the translations of the independent nodes enter the fit, which matches the Nastran default of
components 123 on the independent grids. Their rotations are never used.

The constraint is kinematic: it adds [MultiPointConstraint.md] rows to the `DofMap`. It therefore
stiffens nothing. Use it to introduce a load or a mass at a point without making the region under it
rigid, which is what [RBE2Constraint.md] would do.

### Least-squares fit

Let node $i$ of the independent set carry the weight $w_i$, sit at $\mathbf{x}_i$ and translate by
$\mathbf{u}_i$. Write $W = \sum_i w_i$ for the total weight,
$\mathbf{x}_c = \sum_i w_i \mathbf{x}_i / W$ for the weighted centroid,
$\mathbf{r}_i = \mathbf{x}_i - \mathbf{x}_c$, and
$\mathbf{d} = \mathbf{x}_{\mathrm{ref}} - \mathbf{x}_c$ for the offset of the reference node from
that centroid. All positions come from the undisplaced mesh.

The fit is the rigid-body motion that minimizes the weighted residual
$\sum_i w_i \lvert \mathbf{u}_i - (\bar{\mathbf{u}} + \boldsymbol{\theta} \times \mathbf{r}_i)
\rvert^{2}$. Its solution is

\begin{equation}
\bar{\mathbf{u}} = \frac{1}{W}\sum_i w_i \mathbf{u}_i, \qquad
\boldsymbol{\theta} = J^{-1} \sum_i w_i\, \mathbf{r}_i \times \mathbf{u}_i, \qquad
J = \sum_i w_i \left( \lvert \mathbf{r}_i \rvert^{2} I
    - \mathbf{r}_i \mathbf{r}_i^{\top} \right),
\end{equation}

where $J$ is the weighted second-moment tensor of the independent set. The reference node is
constrained to that motion, evaluated at its own position:

\begin{equation}
\mathbf{u}_{\mathrm{ref}} = \bar{\mathbf{u}} + \boldsymbol{\theta} \times \mathbf{d}, \qquad
\boldsymbol{\theta}_{\mathrm{ref}} = \boldsymbol{\theta} .
\end{equation}

The rotation rows are added only when [!param](/Constraints/RBE3Constraint/rotations) is given. The
three translation rows are always added. Reading the coefficients off the two relations, the
$3\times3$ blocks that node $i$ contributes are

\begin{equation}
\frac{\partial \mathbf{u}_{\mathrm{ref}}}{\partial \mathbf{u}_i}
  = \frac{w_i}{W} I - w_i\, [\mathbf{d}]_\times J^{-1} [\mathbf{r}_i]_\times ,
\qquad
\frac{\partial \boldsymbol{\theta}_{\mathrm{ref}}}{\partial \mathbf{u}_i}
  = w_i\, J^{-1} [\mathbf{r}_i]_\times ,
\end{equation}

with $[\mathbf{a}]_\times$ the skew matrix satisfying
$[\mathbf{a}]_\times \mathbf{b} = \mathbf{a} \times \mathbf{b}$. The weights are relative, not
absolute: scaling every weight by the same factor changes nothing.

### Load distribution

Because a constraint row reduces the load vector by $C^{\top}$, a force $\mathbf{F}$ applied at the
reference node reaches node $i$ as

\begin{equation}
\mathbf{F}_i = \frac{w_i}{W}\,\mathbf{F}
  + w_i \left( J^{-1}(\mathbf{d} \times \mathbf{F}) \right) \times \mathbf{r}_i .
\end{equation}

The first term is the weighted share of the resultant. The second term is the couple that carries
the moment of $\mathbf{F}$ about the centroid, and it vanishes when the reference node sits at the
weighted centroid. The distributed forces sum to $\mathbf{F}$ and produce no net moment about the
reference node.

For the unit square patch with corners $(0,0)$, $(1,0)$, $(1,1)$, $(0,1)$, a reference node at the
patch center $(0.5, 0.5)$ and a unit force along $z$, the table gives the two cases the regression
tests measure. The measured columns come from the truss reactions of
`rbe3_force_split_equal.i`, which the `rbe3_force_split_weighted` test reruns with the weighted
weight set; both agree with the closed form to better than 1e-12.

| weights | $\mathbf{x}_c$ | $\mathbf{d}$ | $F_z$ closed form | $F_z$ measured |
|---|---|---|---|---|
| 1, 1, 1, 1 | $(0.5, 0.5)$ | $(0, 0)$ | $1/4$ at each corner | 0.24999999999942 at each corner |
| 2, 1, 1, 1 | $(0.4, 0.4)$ | $(0.1, 0.1)$ | $2/7,\; 3/14,\; 2/7,\; 3/14$ | 0.28571428571489, 0.21428571428617, 0.28571428571489, 0.21428571428617 |

In the weighted case $W = 5$ and $J^{-1}(\mathbf{d} \times \mathbf{F}) = (1/14, -1/14, 0)$. Dropping
the $\mathbf{d}$ term would give $0.4,\ 0.2,\ 0.2,\ 0.2$ instead.

### Operating envelope

The constraint adds no stiffness, because the reduced stiffness matrix is the original one with the
reference degrees of freedom eliminated, and the reference node carries no element of its own. The
lowest eigenvalue of a clamped plate shifts by only 1.4e-5 relative when an RBE3 constraint over four
interior nodes is added. That residual shift is proportional to the Young's modulus of the soft stub
element that carries the reference node, so it belongs to the stub and not to the constraint.

The object is three-dimensional. [!param](/Constraints/RBE3Constraint/displacements) takes exactly
three variables, and [!param](/Constraints/RBE3Constraint/rotations), when given, takes three as
well.

[!param](/Constraints/RBE3Constraint/reference_boundary) must resolve to exactly one node, and that
node must solve every variable that is constrained on it. A continuum block does not solve
rotations, so a reference node with [!param](/Constraints/RBE3Constraint/rotations) must sit on a
block that does, such as a beam block.

[!param](/Constraints/RBE3Constraint/weights) holds one weight per entry of
[!param](/Constraints/RBE3Constraint/independent_boundaries), and that weight applies to every node
of that node set. Give each node its own node set to give it its own weight. Every weight must be
positive. A size mismatch between the two parameters is an error, and so is a node that appears in
more than one of the listed node sets, because it could then carry two weights.

The rotation of the fit enters the rows only when [!param](/Constraints/RBE3Constraint/rotations) is
given, or when the offset of the reference node from the weighted centroid exceeds a round-off
fraction of the radius of gyration of the independent set. Only then is $J$ inverted, and only then
do independent nodes that are collinear, or a single independent node, raise an error, because $J$
is singular in those cases. A single independent node coincident with the reference node, with
[!param](/Constraints/RBE3Constraint/rotations) unset, therefore remains valid.

Further errors: the reference node listed among the independent nodes, an independent node that
does not solve one of the three displacements, and node sets that hold no nodes at all.

The coefficients are built once from the undisplaced mesh, so the constraint describes small
motions about the reference configuration, and `use_displaced_mesh = true` raises an error.

## Example input syntax

The reference node at the center of a four-truss patch is tied to the four corner nodes with equal
weights. The load is applied to the stub that carries the reference node, and the constraint splits
it onto the corners.

!listing modules/solid_mechanics/test/tests/multipoint_constraints/rbe3/rbe3_force_split_equal.i block=Constraints

!syntax parameters /Constraints/RBE3Constraint

!syntax inputs /Constraints/RBE3Constraint

!syntax children /Constraints/RBE3Constraint

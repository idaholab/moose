# InclinedNoDisplacementConstraint

!syntax description /Constraints/InclinedNoDisplacementConstraint

## Description

`InclinedNoDisplacementConstraint` is the inclined roller: every node of
[!param](/Constraints/InclinedNoDisplacementConstraint/boundary) slides freely in the plane of its
support but does not move through it,

\begin{equation}
\mathbf{u} \cdot \mathbf{n} = 0 ,
\end{equation}

with $\mathbf{u}$ built from the variables listed in
[!param](/Constraints/InclinedNoDisplacementConstraint/displacements), in the order `disp_x disp_y`
in two dimensions and `disp_x disp_y disp_z` in three, and $\mathbf{n}$ the unit normal of the
support at that node.

The condition is applied as one exact constraint per support plane at a node, rather
than as a penalty term. The exact constraint makes one displacement component the dependent one and expresses it
through the others,

!equation id=inclined-exact
u_d = - \sum_{i \neq d} \frac{n_i}{n_d}\, u_i ,

which is the shape of a hanging node constraint. Its right-hand side is zero, so a node on one support
plane slides in that plane through the origin of its own displacement.

Because the exact constraint is applied by the `DofMap`, the condition holds exactly: the measured
$\max |\mathbf{u} \cdot \mathbf{n}|$ on a square rotated by 30 degrees is
$1.4 \times 10^{-20}$, the same value in two and three dimensions, on one, two and three ranks, and
on a replicated or a distributed mesh. On the three-dimensional case where three supports intersect
it is $6 \times 10^{-18}$. No penalty parameter enters, the support adds no stiffness, and the
rotated block reproduces the axis-aligned Dirichlet reference it is rotated from. The rotated square
solves in one nonlinear and one linear iteration.

[MultiPointConstraint.md] is the base class and describes what an exact constraint is, how libMesh
reduces every matrix and vector with it, and what that means under an
[Eigenvalue executioner](EigenProblem.md), where a penalty support reaches neither the stiffness nor
the mass matrix and this constraint is exact out of both.

## The support normal

With [!param](/Constraints/InclinedNoDisplacementConstraint/normal) omitted, the normal of a support
at a node is the area-weighted average of the normals of the faces of +that+ support that meet
there, integrated once over the reference faces when the constraint is built. The normals are held
per support and node, never per node alone. The undisplaced mesh supplies that geometry, which is
the convention [PenaltyInclinedNoDisplacementBC.md] follows as well: its action sets
`use_displaced_mesh = false` on every boundary condition it builds.

Averaging within one support is what makes a corner or an edge node behave as a roller. A node
shared by two faces of the same support receives one averaged normal there, and therefore one exact constraint,
so it stays free to slide along the edge those faces share. One exact constraint per face would instead lock the
node completely.

Because the normal comes from the faces, each boundary has to be a side set. A boundary that holds
no faces, and a node where the faces of one support cancel, the apex of a knife edge for instance,
both raise an error that names
[!param](/Constraints/InclinedNoDisplacementConstraint/normal) as the answer.

[!param](/Constraints/InclinedNoDisplacementConstraint/normal) states the plane of a flat support
directly. It need not be normalized, it may not be the zero vector, and it replaces the mesh normal
at every node of the boundary.

## Several supports at one node

A node that lies on two or more +distinct+ support planes, whether they are two boundaries of one
object or boundaries of separate objects, receives one exact constraint per plane. The exact constraints are triangularized:
before a plane is written as an exact constraint, every degree of freedom of that node which is already dependent
is replaced by the exact constraint that defines it, so each exact constraint ends up depending only on components that no
other exact constraint constrains. libMesh does not reduce its constraints to independent degrees of freedom
until every constraint object has spoken, so the substitution finds the exact constraints of other objects as
well, and two `InclinedNoDisplacementConstraint` objects sharing a node work the same way as one
object with two boundaries.

Three independent supports meeting at a node in three dimensions leave it no direction of motion, so
their three exact constraints pin it. Two supports that name the same plane give one exact constraint and one redundant
equation, which is dropped. Supports whose planes cannot all hold at once, together with whatever
already constrains that node, raise an error saying they have no common direction of motion.

Substitution can also leave an exact constraint with a nonzero right-hand side, when the degree of freedom it
eliminated was itself constrained to a nonzero value. The support plane is then the correct one
through that prescribed motion rather than through the origin.

## Which component carries the exact constraint

The dependent component $d$ of [inclined-exact] is the one with the largest coefficient in magnitude
among the components of the node that are still available, which keeps the coefficients of the exact constraint
bounded. A component is unavailable when an enabled nodal boundary condition pins it, or when it is
already the dependent component of another exact constraint. libMesh applies an exact constraint after the residual
form boundary conditions, so an exact constraint on a pinned component would override the condition; such a
component may still appear on the right-hand side of an exact constraint, where the boundary condition simply
supplies its value.

No exact constraint is written when no component is available, or when the support is orthogonal to every
component that is, because the boundary conditions and the exact constraints already in place decide how the node
moves through that support. A node whose every displacement component is already constrained, a
hanging node for instance, is therefore skipped rather than reported: libMesh's interpolation
already determines that node, and its parents carry the support.

## Operating envelope

The normals are built once, so this is the small deformation, flat or locally flat support. A
support whose normal turns as the body slides over a curved surface is outside it: use
[PenaltyInclinedNoDisplacementBC.md] or a mortar constraint for that case.

The same applies to mesh adaptation. Without
[!param](/Constraints/InclinedNoDisplacementConstraint/normal), a node that refinement adds to the
boundary after the constraint was built has no normal, and the run stops with an error that says so.
With [!param](/Constraints/InclinedNoDisplacementConstraint/normal) given, the plane is a parameter
rather than mesh geometry and covers the new nodes as well.

`use_displaced_mesh = true` is refused, as it is for every [MultiPointConstraint.md]: the
coefficients of an exact constraint are built once from the undisplaced mesh.

## Example Input File Syntax

In this example a rectangular block is rotated out of the coordinate axes and pushed by a pressure
on its `top` side. The `right` and `bottom` sides each rest on an inclined support, and neither
constraint gives [!param](/Constraints/InclinedNoDisplacementConstraint/normal), so each support
plane comes from the faces of its own boundary.

!listing modules/solid_mechanics/test/tests/inclined_bc/inclined_exact_2d.i block=Constraints

!syntax parameters /Constraints/InclinedNoDisplacementConstraint

!syntax inputs /Constraints/InclinedNoDisplacementConstraint

!syntax children /Constraints/InclinedNoDisplacementConstraint

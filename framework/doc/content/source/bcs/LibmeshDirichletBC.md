# LibmeshDirichletBC

## Description

`LibmeshDirichletBC` imposes the essential boundary condition $u = g$ for a constant $g$, sourcing
the prescribed value of every degree of freedom on the boundary from libMesh's Dirichlet constraint
machinery. libMesh derives each value by a local per-entity mass-matrix projection of $g$ onto the
variable's boundary trace space, so what it returns is a coefficient in whatever basis the variable
uses.

That projection is what distinguishes this object from [DirichletBC](DirichletBC.md). `DirichletBC`
assigns $g$ to one degree of freedom per node, as though that degree of freedom were a point value of
the solution. For an interpolatory family such as LAGRANGE it is, and the two objects agree. For a
modal family such as HIERARCHIC above first order it is not: a degree of freedom attached to a
mid-edge node is the coefficient of an edge mode, a shape function that vanishes at the element
vertices, and a node may carry several of them while only the first is assigned for a `DirichletBC`.

A degree of freedom on the boundary that is already constrained by adaptivity (a hanging node) or by
a periodic boundary keeps that constraint.

The boundary condition is a constraint on the discrete system, as a periodic boundary is: it decides
which degrees of freedom are unknowns. It therefore cannot be enabled or disabled during a run.

The [!param](/BCs/LibmeshDirichletBC/boundary) parameter has to name sidesets, since the projection
integrates over element sides.

## Example Input Syntax

In this example, Laplace's equation is solved for a second-order HIERARCHIC variable `u` with
[!param](/BCs/LibmeshDirichletBC/value) set to 1 on every side of the unit square listed in
[!param](/BCs/LibmeshDirichletBC/boundary). The exact solution is $u = 1$ everywhere, and the
projected boundary values reproduce it exactly, which a nodal `DirichletBC` does not on this basis.

!listing test/tests/bcs/libmesh_dirichlet_bc/libmesh_dirichlet_bc.i block=BCs

!syntax parameters /BCs/LibmeshDirichletBC

!syntax inputs /BCs/LibmeshDirichletBC

!syntax children /BCs/LibmeshDirichletBC

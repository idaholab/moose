# LibmeshDirichletBC

!syntax description /BCs/LibmeshDirichletBC

## Description

`LibmeshDirichletBC` imposes the essential boundary condition $u = g$ for a constant $g$, sourcing
the prescribed value of every degree of freedom on the boundary from libMesh's Dirichlet constraint
machinery. libMesh derives each value by a local per-entity mass-matrix projection of $g$ onto the
variable's boundary trace space, so what it returns is a coefficient in whatever basis the variable
uses.

That projection is what distinguishes this object from [DirichletBC.md]. `DirichletBC` assigns $g$ to
one degree of freedom per node, as though that degree of freedom were a point value of the solution.
For an interpolatory family such as LAGRANGE it is, and the two objects agree. For a modal family
such as HIERARCHIC above first order it is not: a degree of freedom attached to a node is a bubble
coefficient, and a node may carry several of them while only the first is assigned.

Enforcement stays with MOOSE, as it does for `DirichletBC`: this object writes its own residual and
Jacobian rows, over every degree of freedom the boundary reaches. libMesh's own constraint
enforcement is not used, because the nonlinear solver enforces constraints in their homogeneous form,
which carries an adaptivity hanging-node or periodic constraint but drops a prescribed value.

A degree of freedom that libMesh already constrains on its own is left to that constraint, rather
than being prescribed twice.

The `boundary` parameter has to name sidesets, since the projection integrates over element sides.

## Example Input Syntax

!listing test/tests/bcs/libmesh_dirichlet_bc/libmesh_dirichlet_bc.i block=BCs

!syntax parameters /BCs/LibmeshDirichletBC

!syntax inputs /BCs/LibmeshDirichletBC

!syntax children /BCs/LibmeshDirichletBC

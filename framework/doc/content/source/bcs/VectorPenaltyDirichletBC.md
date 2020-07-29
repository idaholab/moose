# VectorPenaltyDirichletBC

!syntax description /BCs/VectorPenaltyDirichletBC

## Description

`VectorPenaltyDirichletBC` implements a vector form of [`PenaltyDirichletBC`](/PenaltyDirichletBC.md),
which enforces a weak sense of the Dirichlet boundary
condition by setting the boundary as a penalty times the inner product of the
test function and the difference between the current solution and Dirichlet
data. This is given by $p(\vec{u}^\ast, \vec{u} - \vec{u}_0)$, where $p$ is a
scalar defining the penalty value, $\vec{u}^\ast$ is the test function, and
$\vec{u} - \vec{u}_0$ is the vector difference between the current solution and
Dirichlet data.

This boundary condition can be useful for problems where the mesh is not as
refined and could potentially smooth out the problem data on a coarser mesh. It,
however, has problems on refined meshes and leads to an ill-conditioned problem,
which can be difficult to solve.

## Example Input Syntax

!listing test/tests/kernels/vector_fe/electromagnetic_coulomb_gauge.i block=BCs

!syntax parameters /BCs/VectorPenaltyDirichletBC

!syntax inputs /BCs/VectorPenaltyDirichletBC

!syntax children /BCs/VectorPenaltyDirichletBC

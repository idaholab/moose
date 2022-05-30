# VectorCurlPenaltyDirichletBC

!syntax description /BCs/VectorCurlPenaltyDirichletBC

## Overview

`VectorCurlPenaltyDirichletBC` enforces a weak sense of the Dirichlet boundary
condition of the curl of the nonlinear variable by setting the boundary as a
penalty times the inner product of the test function crossed with the surface
normal and the difference between the current solution and the Dirichlet data,
also crossed with the surface normal. This is given by

\begin{equation}
  p(\vec{u}^\ast \times \hat{n}, (\vec{u} - \vec{u}_0) \times \hat{n})
\end{equation}

where $p$ is a scalar defining the penalty value, $\vec{u}^\ast$ is the test
function, and $\vec{u} - \vec{u}_0$ is the vector difference between the current
solution and Dirichlet data.

This boundary condition can be useful for problems where the mesh is not as
refined and could potentially smooth out the problem data on a coarser mesh. It,
however, has problems on refined meshes and leads to an ill-conditioned problem,
which can be difficult to solve.

## Example Input File Syntax

!listing test/tests/kernels/vector_fe/vector_kernel.i block=BCs

!syntax parameters /BCs/VectorCurlPenaltyDirichletBC

!syntax inputs /BCs/VectorCurlPenaltyDirichletBC

!syntax children /BCs/VectorCurlPenaltyDirichletBC

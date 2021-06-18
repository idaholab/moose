# VectorCurlPenaltyDirichletBC

!syntax description /BCs/VectorCurlPenaltyDirichletBC

## Overview

The VectorCurlPenaltyDirichletBC object is a VectorIntegratedBC which uses a
penalty method to set the following Dirichlet condition in the context of a
vector field Helmholtz wave equation:

\begin{equation}
  \vec{E} = \vec{E}_{exact}
\end{equation}

where

- $\vec{E}$ is the electric field solution vector, and
- $\vec{E}_{exact}$ is the exact electric field solution.

This can be weakly imposed on a boundary by using the following inner product
residual contribution to the finite element equation:

\begin{equation}
  P \left( (\vec{E} - \vec{E}_{exact}) \times \hat{\mathbf{n}}, \vec{\psi}_i \times \hat{\mathbf{n}}) \right)
\end{equation}

where

- $\vec{\psi}_i$ is the test function,
- $P$ is the scalar penalty value, and
- $\hat{\mathbf{n}}$ is the boundary normal vector.

## Example Input File Syntax

!listing vector_kernels.i block=BCs/sides

!syntax parameters /BCs/VectorCurlPenaltyDirichletBC

!syntax inputs /BCs/VectorCurlPenaltyDirichletBC

!syntax children /BCs/VectorCurlPenaltyDirichletBC

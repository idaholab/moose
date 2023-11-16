# VectorDivPenaltyDirichletBC

!syntax description /BCs/VectorDivPenaltyDirichletBC

## Overview

`VectorDivPenaltyDirichletBC` enforces, in a weak sense, a Dirichlet boundary
condition on the divergence of a nonlinear vector variable $\vec{u}$ by setting

\begin{equation}
  R_i(\vec{u}) = p($\vec{\psi_i}$ \cdot \hat{n}, (\vec{u} - \vec{u}_0) \cdot \hat{n})
\end{equation}

where $p$ is a scalar defining the penalty value, $\vec{\psi_i}$ the test
functions, and $\vec{u} - \vec{u}_0$ is the vector difference between the
current solution and the Dirichlet data.

## Example Input File Syntax

!listing coupled_electrostatics.i block=BCs

!syntax parameters /BCs/VectorDivPenaltyDirichletBC

!syntax inputs /BCs/VectorDivPenaltyDirichletBC

!syntax children /BCs/VectorDivPenaltyDirichletBC

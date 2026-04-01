# DivDivField

## Overview

!style halign=left
The DivDivField object implements the following PDE term for vector variables:

\begin{equation}
  - \nabla \left( k \nabla \cdot \vec{u} \right)
\end{equation}

where $k$ is a constant scalar coefficient and $\vec{u}$ is a vector field
variable. Given vector test functions $\vec{\psi_i}$, the weak form, in
inner-product notation, is given by:

\begin{equation}
  R_i(\vec{u}) = (\nabla \cdot \vec{\psi_i}, k \nabla \cdot \vec{u}) \quad \forall \vec{\psi_i}.
\end{equation}

## Example Input File Syntax

!listing grad_div.i block=Kernels/divergence

!syntax parameters /Kernels/DivDivField

!syntax inputs /Kernels/DivDivField

!syntax children /Kernels/DivDivField

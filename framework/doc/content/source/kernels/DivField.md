# DivField

!syntax description /Kernels/DivField

## Overview

!style halign=left
The DivField object implements the following PDE term for coupled scalar-vector
PDE systems:

\begin{equation}
  k \nabla \cdot \vec{u},
\end{equation}

where $k$ is a constant scalar coefficient and $\vec{u}$ is a vector field
variable. Given scalar test functions $\psi_i$, the weak form, in inner-product notation, is given by:

\begin{equation}
  R_i(\vec{u}) = (\psi_i, k \nabla \cdot \vec{u}) \quad \forall \psi_i.
\end{equation}

## Example Input File Syntax

!listing coupled_electrostatics.i block=Kernels/divergence

!syntax parameters /Kernels/DivField

!syntax inputs /Kernels/DivField

!syntax children /Kernels/DivField

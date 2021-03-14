# PEC

!syntax description /BCs/PEC

## Overview

The PEC object is an implementation of the Perfect Electrical Conductor Boundary
condition for scalar variables, which seeks the minimization of the inner product
residual contribution

\begin{equation}
  -(\hat{\mathbf{n}} \times \vec{\psi}_i \text{  ,  } \hat{\mathbf{n}} \times \vec{E})
\end{equation}

where

- $\vec{E}$ is the electric field vector,
- $\vec{\psi}_i$ is a vector which contains a scalar basis for each component, and
- $\hat{\mathbf{n}}$ is the normal vector at the boundary.

Note again that this object is made for scalar variables, which means that $\vec{E}$
is a constructed vector made up of coupled variables representing the components
of the vector field (`coupled_0`, `coupled_1`, and `coupled_2`, respectively).

## Example Input File Syntax

!alert warning title=This is not currently tested
The PEC object is not currently used in any tested input files. This
section of the documentation will be updated when this occurs. See a selection of
untested input files where this is used at the bottom of this page.

!! TODO: add a test to fix this up!

!syntax parameters /BCs/PEC

!syntax inputs /BCs/PEC

!syntax children /BCs/PEC

# VectorSecondTimeDerivative

!syntax description /Kernels/VectorSecondTimeDerivative

## Overview

!style halign=left
The VectorSecondTimeDerivative object implements the weak form inner product term
associated with the second time derivative of a vector field variable with a function
coefficient. The term is

\begin{equation}
  \left(\vec{\psi}_i \; , \; a(\mathbf{r}, t) \frac{\partial^2 \vec{u}}{\partial t^2}\right)
\end{equation}

where

- $\vec{\psi}_i$ is a vector-valued test function,
- $\vec{u}$ is the solution vector field variable, and
- $a(\mathbf{r}, t)$ is a function coefficient (default = 1.0).

## Example Input File Syntax

!listing dipole_transient.i block=Kernels/time_derivative_real

!syntax parameters /Kernels/VectorSecondTimeDerivative

!syntax inputs /Kernels/VectorSecondTimeDerivative

!syntax children /Kernels/VectorSecondTimeDerivative

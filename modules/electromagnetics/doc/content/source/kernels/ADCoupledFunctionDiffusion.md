# ADCoupledFunctionDiffusion

!syntax description /Kernels/ADCoupledFunctionDiffusion

## Overview

!style halign=left
The ADCoupledFunctionDiffusion object represents a coupled Laplacian operator term with a function coefficient for scalar field variables. This term is

\begin{equation}
  s f(\mathbf{r}, t) \nabla^2 v
\end{equation}

where

- $v$ is a coupled scalar field variable,
- $f(\mathbf{r}, t)$ is a scalar function serving as a coefficient, and
- $s$ is a constant coefficient providing the sign of the term in the PDE (default = 1.0).

## Example Input File Syntax

!listing scalar_complex_helmholtz.i block=Kernels/coupledLaplacian_real

!syntax parameters /Kernels/ADCoupledFunctionDiffusion

!syntax inputs /Kernels/ADCoupledFunctionDiffusion

!syntax children /Kernels/ADCoupledFunctionDiffusion

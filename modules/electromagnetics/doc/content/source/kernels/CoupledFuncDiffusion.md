# CoupledFuncDiffusion

!syntax description /Kernels/CoupledFuncDiffusion

## Overview

!style halign=left
The CoupledFuncDiffusion object represents a coupled Laplacian operator term with a function coefficient for scalar variables. This term is

\begin{equation}
  s f(\mathbf{r}) \nabla^2 v
\end{equation}

where

- $v$ is a coupled scalar variable,
- $f(\mathbf{r})$ is a scalar function serving as a coefficient, and
- $s$ is a constant coefficient providing the sign of the term in the PDE (default = 1.0).

## Example Input File Syntax

!listing coupled_diffusion_helmholtz.i block=Kernels/coupledLaplacian_real

!syntax parameters /Kernels/CoupledFuncDiffusion

!syntax inputs /Kernels/CoupledFuncDiffusion

!syntax children /Kernels/CoupledFuncDiffusion

# FuncDiffusion

!syntax description /Kernels/FuncDiffusion

## Overview

!style halign=left
The FuncDiffusion object represents a Laplacian operator term with a function coefficient for scalar variables. This term is

\begin{equation}
  f(\mathbf{r}) \nabla^2 u
\end{equation}

where

- $u$ is a scalar solution variable, and
- $f(\mathbf{r})$ is a scalar function serving as a coefficient.

## Example Input File Syntax

!listing coupled_diffusion_helmholtz.i block=Kernels/laplacian_real

!syntax parameters /Kernels/FuncDiffusion

!syntax inputs /Kernels/FuncDiffusion

!syntax children /Kernels/FuncDiffusion

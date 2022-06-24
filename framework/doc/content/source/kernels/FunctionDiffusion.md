# FunctionDiffusion

!syntax description /Kernels/FunctionDiffusion

## Overview

!style halign=left
The `FunctionDiffusion` object represents a Laplacian operator term with a function coefficient for scalar field variables. This term is

\begin{equation}
  f(\mathbf{r}, t) \nabla^2 u
\end{equation}

where

- $u$ is a scalar field solution variable, and
- $f(\mathbf{r}, t)$ is a scalar function serving as a coefficient.

## Example Input File Syntax

!listing function_diffusion.i block=Kernels/diff

!syntax parameters /Kernels/FunctionDiffusion

!syntax inputs /Kernels/FunctionDiffusion

!syntax children /Kernels/FunctionDiffusion

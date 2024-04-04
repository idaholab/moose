# FunctionDiffusion

!syntax description /Kernels/FunctionDiffusion

## Overview

!style halign=left
The `FunctionDiffusion` object represents a diffusion term with a function coefficient for
scalar field variables. This term is

\begin{equation}
  - \nabla \cdot f(\mathbf{r}, t) \nabla v
\end{equation}

where

- $v$ is a scalar field solution variable, and
- $f(\mathbf{r}, t)$ is a scalar function serving as a diffusion coefficient.

Note that the [!param](/Kernels/FunctionDiffusion/v) parameter is optional. If no variable is entered 
there, then the kernel's nonlinear variable will be used in the operator as usual. 

## Example Input File Syntax

!listing function_diffusion.i block=Kernels/diff

!syntax parameters /Kernels/FunctionDiffusion

!syntax inputs /Kernels/FunctionDiffusion

!syntax children /Kernels/FunctionDiffusion

# ADDiffusionRate

## Description

Computes the Laplacian of the time rate of change of a variable with a viscosity $\mu$:
\begin{equation}
-\nabla \cdot \mu \nabla \dot{u}
\end{equation}

The Jacobian in `ADDiffusionRate` is computed using forward automatic
differentiation.

!syntax parameters /Kernels/ADDiffusionRate

!syntax inputs /Kernels/ADDiffusionRate

!syntax children /Kernels/ADDiffusionRate

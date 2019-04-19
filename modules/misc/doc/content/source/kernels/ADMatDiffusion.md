# ADMatDiffusion

## Description

`ADMatDiffusion` implements the term
\begin{equation}
\nabla D(a,b,\dots) \nabla c,
\end{equation}
where the diffusion coefficient $D$ (`diffusivity`) is provided by a `Material` or `ADMaterial`.
$D$ can depend on arbitrary non-linear variables $a,b,\dots$ (`args`).
The complete Jacobian contributions are provided by automatic differentiation as long as $D$ is given using an `ADMaterial` derived object.

This class inherits from the [ADDiffusion](/ADDiffusion.md) class.

!syntax parameters /Kernels/ADMatDiffusion

!syntax inputs /Kernels/ADMatDiffusion

!syntax children /Kernels/ADMatDiffusion

!bibtex bibliography

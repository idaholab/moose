# ADMatDiffusion

## Description

`ADMatDiffusion` implements the term
\begin{equation}
\nabla\cdot D(a,b,\dots) \nabla u,
\end{equation}
where the diffusion coefficient $D$ (`diffusivity`) is provided by a `Material` or
`ADMaterial` and $u$ is the nonlinear variable the kernel is operating on.

This kernel can be used in a coupled form if the optional `v` variable is
specified. This allows applying the diffusion operator to a variable $u$ given
by `v`, which is different from the kernel variable.

$D$ can depend on arbitrary nonlinear variables $a,b,\dots$. The complete
Jacobian contributions are provided by automatic differentiation as long as $D$
is given using an `ADMaterial` derived object.

!syntax parameters /Kernels/ADMatDiffusion

!syntax inputs /Kernels/ADMatDiffusion

!syntax children /Kernels/ADMatDiffusion

!bibtex bibliography

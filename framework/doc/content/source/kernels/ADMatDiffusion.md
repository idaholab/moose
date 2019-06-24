# ADMatDiffusion

## Description

`ADMatDiffusion` implements the term
\begin{equation}
\nabla D(a,b,\dots) \nabla c,
\end{equation}
where the diffusion coefficient $D$ (`D_name`) is provided by a `Material` or
`ADMaterial` and $c$ is either a coupled variable (`conc`) or - if not
explicitly specified - the non-linear variable the kernel is operating on. $D$
can depend on arbitrary non-linear variables $a,b,\dots$ (`args`). The complete
Jacobian contributions are provided by automatic differentiation as long as $D$
is given using an `ADMaterial` derived object.

!syntax parameters /Kernels/ADMatDiffusion

!syntax inputs /Kernels/ADMatDiffusion

!syntax children /Kernels/ADMatDiffusion

!bibtex bibliography

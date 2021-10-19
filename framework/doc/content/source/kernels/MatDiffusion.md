# MatDiffusion

!syntax description /Kernels/MatDiffusion

Implements the term
\begin{equation}
\nabla\cdot D(c,a,b,\dots) \nabla u,
\end{equation}
where the diffusion coefficient $D$ (`diffusivity`) is provided by a `FunctionMaterial` function material (see `Phase Field Module` for more information), $u$ is the nonlinear variable the kernel is operating on.  

This kernel can be used in a coupled form if the optional `v` variable is
specified. This allows applying the diffusion operator to a variable $u$ given
by `v`, which is different from the kernel variable.

$D$ can depend on arbitrary nonlinear variables $a,b,\dots$ (`args`). The
complete Jacobian contributions are provided by the kernel. To build the
Jacobian the kernel uses all derivatives of $D$ with respect to the kernel
variable and the variables specified in `args`.

!syntax parameters /Kernels/MatDiffusion

!syntax inputs /Kernels/MatDiffusion

!syntax children /Kernels/MatDiffusion

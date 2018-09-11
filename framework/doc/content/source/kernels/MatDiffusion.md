# MatDiffusion

!syntax description /Kernels/MatDiffusion

Implements the term
\begin{equation}
\nabla D(c,a,b,\dots) \nabla c,
\end{equation}
where the diffusion coefficient $D$ (`D_name`) is provided by a `FunctionMaterial` function material (see `Phase Field Module` for more information), $c$ is either a coupled variable (`conc`)
or - if not explicitly specified - the non-linear variable the kernel is operating on.
$D$ can depend on arbitrary non-linear variables $a,b,\dots$ (`args`).
The complete Jacobian contributions are provided by the kernel.

!syntax parameters /Kernels/MatDiffusion

!syntax inputs /Kernels/MatDiffusion

!syntax children /Kernels/MatDiffusion

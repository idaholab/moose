# PolycrystalMatAnisoDiffusion

!syntax description /Kernels/PolycrystalMatAnisoDiffusion

This is anisotropic version of [`MatDiffusion`](/MatDiffusion.md), which expects
a tensor-valued diffusion coefficient $D$ (`diffusivity`). It is the same as
[`MatAnisoDiffusion`](/MatAnisoDiffusion.md) except that it accounts for explicit
dependence of $D$ on order parameters and the gradients of the order parameters in calculating the Jacobian contributions. This is in contrast to [`MatAnisoDiffusion`](/MatAnisoDiffusion.md), where only explicit dependence of $D$ on the order parameters themselves is accounted for. Accounting for the Jacobian contribution can accelerate convergence when $D$ explicitly depends on gradients of the order parameters. An example of a case where $D$ depends explicitly on gradients of the order parameters is [`PolycrystalDiffusivityTensorBase`](/PolycrystalDiffusivityTensorBase.md).

The off-diagonal Jacobian contribution is calculated as
\begin{equation}
\frac{\partial R_i}{\partial u_j} = \nabla \psi \cdot \left(\frac{\partial D}{\partial u_h} \frac{\partial u_h}{\partial u_j} + \frac{\partial D}{\partial \nabla u_h} \frac{\partial \nabla u_h}{\partial u_j} \right) \nabla v
\end{equation}
\begin{equation}
\frac{\partial R_i}{\partial u_j} = \nabla \psi \cdot \left(\frac{\partial D}{\partial u} \phi_j + \frac{\partial D}{\partial \nabla u} \nabla \phi_j \right) \nabla v
\end{equation}
where $R_i$ is the residual contribution from this kernel, $u_j$ is the basis function coefficient, $\psi$ is the test function, $u_h$ is the trial function, $u$ is the variable that $u_h$ is a discrete approximation of, $\phi_j$ is the basis or shape function, and $v$ is the non-linear variable for this kernel. The first term in the above equation is accounted for in [`MatAnisoDiffusion`](/MatAnisoDiffusion.md). The second term is accounted for in this kernel.

!syntax parameters /Kernels/PolycrystalMatAnisoDiffusion

!syntax inputs /Kernels/PolycrystalMatAnisoDiffusion

!syntax children /Kernels/PolycrystalMatAnisoDiffusion

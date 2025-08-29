# PolycrystalMatDiffusion

!syntax description /Kernels/PolycrystalMatDiffusion

This kernel implements the same term as [`MatDiffusion`](/MatDiffusion.md) (residual contribution is the same). However, this kernel can account for explicit
dependence of $D$ on order parameters and the gradients of the order parameters in calculating the Jacobian contributions. This is in contrast to [`MatDiffusion`](/MatDiffusion.md), where only explicit dependence of $D$ on the order parameters themselves is accounted for. Accounting for the Jacobian contribution can accelerate convergence when $D$ explicitly depends on gradients of the order parameters.

The off-diagonal Jacobian contribution is calculated as
\begin{equation}
\frac{\partial R_i}{\partial u_j} = \nabla \psi \cdot \left(\frac{\partial D}{\partial u_h} \frac{\partial u_h}{\partial u_j} + \frac{\partial D}{\partial \nabla u_h} \frac{\partial \nabla u_h}{\partial u_j} \right) \nabla v
\end{equation}
\begin{equation}
\frac{\partial R_i}{\partial u_j} = \nabla \psi \cdot \left(\frac{\partial D}{\partial u} \phi_j + \frac{\partial D}{\partial \nabla u} \nabla \phi_j \right) \nabla v
\end{equation}
where $R_i$ is the residual contribution from this kernel, $u_j$ is the basis function coefficient, $\psi$ is the test function, $u_h$ is the trial function, $u$ is the variable that $u_h$ is a discrete approximation of, $\phi_j$ is the basis or shape function, and $v$ is the non-linear variable for this kernel. The first term in the above equation is accounted for in [`MatDiffusion`](/MatDiffusion.md). The second term is accounted for in this kernel.

!syntax parameters /Kernels/PolycrystalMatDiffusion

!syntax inputs /Kernels/PolycrystalMatDiffusion

!syntax children /Kernels/PolycrystalMatDiffusion

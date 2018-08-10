# ACInterface2DMultiPhase2

!syntax description /Kernels/ACInterface2DMultiPhase2

Implements the Allen-Cahn term for the $\frac{\kappa}2 \sum |\nabla \eta_{\alpha i}|^2$ gradient
energy contribution for the isotropic mobility case. Its weak form is
\begin{equation}
\left( \kappa \nabla \eta_{\alpha i}, \nabla (L \psi) \right),
\end{equation}
where $\kappa_i$ (`kappa_name`) is the gradient energy coefficent, $\eta_i$ the non-conserved
non-linear order parameter variable the kernel is acting on, $L$ (`mob_name`) is
the scalar (isotropic) mobility associated with the order parameter, and $\psi$
is the test function. It is assumed $\kappa$ is a function of $\nabla \eta_{\alpha}$
and $\nabla \eta_{\beta}$.Therefore, kappa depends on multiple order parameters.

!syntax parameters /Kernels/ACInterface2DMultiPhase2

!syntax inputs /Kernels/ACInterface2DMultiPhase2

!syntax children /Kernels/ACInterface2DMultiPhase2

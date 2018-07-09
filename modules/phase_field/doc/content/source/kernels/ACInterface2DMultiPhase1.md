# ACInterface2DMultiPhase1

!syntax description /Kernels/ACInterface2DMultiPhase1

Implements the Allen-Cahn term for the $\frac{\kappa}2 \sum |\nabla \eta_{\alpha i}|^2$ gradient
energy contribution for the anisotropic interface energy case. Its weak form is
\begin{equation}
\left(\nabla (L \psi), \frac{1}{2} \frac {\partial \kappa} {\partial \nabla \eta_{\alpha i}} \sum (\nabla \eta_{\beta j})^2  \right),
\end{equation}
where $\kappa$ (`kappa_name`) is the gradient energy coefficent, $\eta_{\alpha i}$ the non-conserved
non-linear order parameter variable the kernel is acting on, $L$ (`mob_name`) is
the scalar (isotropic) mobility associated with the order parameter, and $\psi$
is the test function.

!syntax parameters /Kernels/ACInterface2DMultiPhase1

!syntax inputs /Kernels/ACInterface2DMultiPhase1

!syntax children /Kernels/ACInterface2DMultiPhase1

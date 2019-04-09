# ADACInterface

!syntax description /Kernels/ADACInterface<RESIDUAL>

Implements the Allen-Cahn term for the $\frac{\kappa_i}2|\nabla \eta_i|^2$
gradient energy contribution for the isotropic mobility case. Its weak form is

\begin{equation}
\left( \kappa_i \nabla \eta_i, \nabla \cdot (L_i \psi_m ) \right),
\end{equation}

where $\kappa_i$ (`kappa_name`) is the gradient energy coefficent, $\eta_i$ the
non-conserved non-linear order parameter variable the kernel is acting on, $L_i$
(`mob_name`) is the scalar (isotropic) mobility associated with the order
parameter, and $\psi_m$ is the test function.

!syntax parameters /Kernels/ADACInterface<RESIDUAL>

!syntax inputs /Kernels/ADACInterface<RESIDUAL>

!syntax children /Kernels/ADACInterface<RESIDUAL>

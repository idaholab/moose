# ADACInterface

!syntax description /Kernels/ADACInterface

Implements the Allen-Cahn term for the $\frac{\kappa_i}2|\nabla \eta_i|^2$
gradient energy contribution for the isotropic mobility case. Its weak form is

\begin{equation}
\left( \kappa_i \nabla \eta_i, \nabla (L_i \psi_m ) \right),
\end{equation}

where $\kappa_i$ (`kappa_name`) is the gradient energy coefficient, $\eta_i$ the
non-conserved non-linear order parameter variable the kernel is acting on, $L_i$
(`mob_name`) is the scalar (isotropic) mobility associated with the order
parameter, and $\psi_m$ is the test function.

!syntax parameters /Kernels/ADACInterface

!syntax inputs /Kernels/ADACInterface

!syntax children /Kernels/ADACInterface

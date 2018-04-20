# SimpleCHInterface

!syntax description /Kernels/SimpleCHInterface

!include simple_kernels_include.md

This kernel implements the weak form
\begin{equation}
(\kappa M \nabla^2 u, \nabla^2\psi)
\end{equation}
for the gradient contribution in the Cahn-Hilliard equation. For a more feature complete
version see the [`CHInterface`](/CHInterface.md) kernel, which allows for variable
dependent mobilities $M$ and gradient energy parameters $\kappa$.

This kernel requires the use of high order shape functions (e.g. third order Hermite).

!syntax parameters /Kernels/SimpleCHInterface

!syntax inputs /Kernels/SimpleCHInterface

!syntax children /Kernels/SimpleCHInterface

!bibtex bibliography

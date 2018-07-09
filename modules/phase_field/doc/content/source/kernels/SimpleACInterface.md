# SimpleACInterface

!syntax description /Kernels/SimpleACInterface

!include simple_kernels_include.md

This kernel implements the weak form
\begin{equation}
(\kappa L \nabla u, \nabla\psi)
\end{equation}
for the gradient contribution in the Allen-Cahn equation. For a more feature complete
version see the [`ACInterface`](/ACInterface.md) kernel, which allows for variable
dependent mobilities $M$ and gradient energy parameters $\kappa$.

!syntax parameters /Kernels/SimpleACInterface

!syntax inputs /Kernels/SimpleACInterface

!syntax children /Kernels/SimpleACInterface

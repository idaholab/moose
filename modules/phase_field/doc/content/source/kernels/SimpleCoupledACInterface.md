# SimpleCoupledACInterface

!syntax description /Kernels/SimpleCoupledACInterface

!include simple_kernels_include.md

This kernel implements the weak form
\begin{equation}
(\kappa L \nabla \eta, \nabla\psi)
\end{equation}
for the gradient contribution in a coupled Allen-Cahn equation.
For the more common case where $\eta$ is the variable the kernel is acting on see
[`SimpleACInterface`](/SimpleACInterface.md).

!syntax parameters /Kernels/SimpleCoupledACInterface

!syntax inputs /Kernels/SimpleCoupledACInterface

!syntax children /Kernels/SimpleCoupledACInterface

!bibtex bibliography

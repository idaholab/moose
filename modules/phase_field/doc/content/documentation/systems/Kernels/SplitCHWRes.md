# SplitCHWRes

!syntax description /Kernels/SplitCHWRes

This kernel implements the weak form
\begin{equation}
(M \nabla u, \nabla\psi)
\end{equation}
for the a reverse split Cahn-Hilliard equation, where the kernel variable $u$ is a chemical potential
and $M$ (`mob_name`) is a scalar mobility.
It is used together with [`SplitCHParsed`](/SplitCHParsed.md) and [`CoupledTimeDerivative`](/CoupledTimeDerivative.md)
to set up a system of two first order PDEs using a concentration order parameter and a chemical potential variable.

For an implementation with an anisotropic (tensorial) mobility see [`SplitCHWResAniso`](/SplitCHWResAniso.md).

!syntax parameters /Kernels/SplitCHWRes

!syntax inputs /Kernels/SplitCHWRes

!syntax children /Kernels/SplitCHWRes

!bibtex bibliography

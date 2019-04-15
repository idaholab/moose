# ADSplitCHWRes

!syntax description /Kernels/ADSplitCHWRes

This kernel implements the weak form

\begin{equation}
(M \nabla u, \nabla\psi)
\end{equation}

for the a reverse split Cahn-Hilliard equation, where the kernel variable $u$ is
a chemical potential and $M$ (`mob_name`) is a scalar mobility. It is used
together with [`ADSplitCHParsed`](/ADSplitCHParsed.md) and
[`ADCoupledTimeDerivative`](/ADCoupledTimeDerivative.md) to set up a system of
two first order PDEs using a concentration order parameter and a chemical
potential variable.

For an implementation with an anisotropic (tensorial) mobility see
[`ADSplitCHWResAniso`](/ADSplitCHWResAniso.md).

!syntax parameters /Kernels/ADSplitCHWRes

!syntax inputs /Kernels/ADSplitCHWRes

!syntax children /Kernels/ADSplitCHWRes

!bibtex bibliography

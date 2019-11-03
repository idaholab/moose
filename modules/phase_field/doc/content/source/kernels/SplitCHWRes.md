# SplitCHWRes

!syntax description /Kernels/SplitCHWRes

This kernel implements the weak form
\begin{equation}
(M \nabla u, \nabla\psi)
\end{equation}
for the a reverse split Cahn-Hilliard equation, where the kernel variable $u$ is
a chemical potential and $M$ (`mob_name`) is a scalar mobility. It is used
together with [`SplitCHParsed`](/SplitCHParsed.md) and
[`CoupledTimeDerivative`](/CoupledTimeDerivative.md) to set up a system of two
first order PDEs using a concentration order parameter and a chemical potential
variable.

## Off-diagonal Onsager Matrix

The terms containing the off-diagonal components of the Onsager
phenomenological transport coefficients model the driving force resulting from
coupled order parameters. These terms are implemented using this kernel by
specifying a different coupled chemical potential variable $w$ than the kernel
variable using the `w` parameter.
\begin{equation}
(M \nabla w, \nabla\psi)
\end{equation}

The strong form of such a coupled system with two variables $c_1$ and $c_2$
would be
\begin{equation}
\begin{aligned}
\frac{\partial c_1}{\partial t} &= \nabla\cdot M_{11} \nabla \frac{\delta F}{\delta c_1} + \nabla\cdot  M_{12} \nabla \frac{\delta F}{\delta c_2}\\
\frac{\partial c_2}{\partial t} &= \nabla\cdot  M_{21} \nabla \frac{\delta F}{\delta c_1} + \nabla\cdot  M_{22} \nabla \frac{\delta F}{\delta c_2}.
\end{aligned}
\end{equation}

And the MOOSE implementation would be

!listing modules/phase_field/test/tests/phase_field_kernels/SplitCHWRes.i block=Kernels


For an implementation with an anisotropic (tensorial) mobility see
[`SplitCHWResAniso`](/SplitCHWResAniso.md).

!syntax parameters /Kernels/SplitCHWRes

!syntax inputs /Kernels/SplitCHWRes

!syntax children /Kernels/SplitCHWRes

!bibtex bibliography

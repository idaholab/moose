# LevelSetForcingFunctionSUPG

This kernel adds the Streamline Upwind/Petrov-Galerkin (SUPG) stabilization
term [!citep](brooks1982streamline, donea2003finite)  to a forcing or source term of a partial differential equation:

\begin{equation}
\label{eq:LevelSetForcingFunctionSUPG:weak}
\left(-\tau \vec{v} \psi_i,\, f\right) = 0,
\end{equation}
where $\vec{v}$ is the level set velocity, $f$ is the forcing term, and $\tau$ as defined below.

\begin{equation}
\label{eq:LevelSetForcingFunctionSUPG:tau}
\tau = \frac{h}{2\|\vec{v}\|},
\end{equation}
where $h$ is the element length.

## Example Syntax

The LevelSetForcingFunctionSUPG [Kernel](syntax/Kernels/index.md) should be used in conjunction with a forcing term. For
example, if a [BodyForce](/BodyForce.md) is defined as follows in the `[Kernels]` block.

!listing modules/level_set/test/tests/verification/1d_level_set_supg_mms/1d_level_set_supg_mms.i block=phi_forcing

Given the forcing term, it is then possible to add the SUPG term to the same forcing function in the `[Kernels]` block
as follows.

!listing modules/level_set/test/tests/verification/1d_level_set_supg_mms/1d_level_set_supg_mms.i block=phi_forcing_supg


!syntax parameters /Kernels/LevelSetForcingFunctionSUPG

!syntax inputs /Kernels/LevelSetForcingFunctionSUPG

!syntax children /Kernels/LevelSetForcingFunctionSUPG



!bibtex bibliography

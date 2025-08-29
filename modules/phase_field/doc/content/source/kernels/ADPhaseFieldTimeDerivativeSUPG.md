# ADPhaseFieldTimeDerivativeSUPG

This kernel adds the Streamline Upwind/Petrov-Galerkin (SUPG) stabilization
term [!citep](brooks1982streamline, donea2003finite)  to the time term of the phase field equation.

\begin{equation}
\label{eq:ADPhaseFieldTimeDerivativeSUPG:weak}
\left(-\tau \vec{v} \psi_i,\, \frac{\partial \phi_h}{\partial t}\right) = 0,
\end{equation}
where $\vec{v}$ is the phase field velocity, $f$ is the forcing term, and $\tau$ as defined below.

\begin{equation}
\label{eq:PhaseFieldTimeDerivativeSUPGtUPG:tau}
\tau = \frac{h}{2\|\vec{v}\|},
\end{equation}
where $h$ is the element length.

## Example Syntax

The ADPhaseFieldTimeDerivativeSUPG [Kernel](syntax/Kernels/index.md) should be used in conjunction with the time term of the phase field equation.
For example, the following provides the necessary objects for phase field the time derivative term with SUPG stabilization.

!listing modules/phase_field/test/tests/phase_field_advection/phase_field_supg_mms.i block=Kernels

!syntax parameters /Kernels/ADPhaseFieldTimeDerivativeSUPG

!syntax inputs /Kernels/ADPhaseFieldTimeDerivativeSUPG

!syntax children /Kernels/ADPhaseFieldTimeDerivativeSUPG



!bibtex bibliography

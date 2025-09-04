# ADPhaseFieldAdvectionSUPG

This kernel adds the Streamline Upwind/Petrov-Galerkin (SUPG) stabilization
term [!citep](brooks1982streamline, donea2003finite) to the advection term of phase field equation.

\begin{equation}
\label{eq:ADPhaseFieldAdvectionSUPG:weak}
\left(-\tau \vec{v} \psi_i,\, \vec{v}\cdot\nabla \phi_h\right) = 0,
\end{equation}
where $\vec{v}$ is the phase field velocity, $f$ is the forcing term, and $\tau$ as defined below.

\begin{equation}
\label{eq:ADPhaseFieldAdvectionSUPG:tau}
\tau = \frac{h}{2\|\vec{v}\|},
\end{equation}
where $h$ is the element length.

## Example Syntax

The ADPhaseFieldAdvectionSUPG [Kernel](syntax/Kernels/index.md) should be used in conjunction with the phase field advection term.
For example, the following provides the necessary objects for the phase field advection term with SUPG stabilization.

!listing modules/phase_field/test/tests/phase_field_advection/phase_field_supg_mms.i block=Kernels


!syntax parameters /Kernels/ADPhaseFieldAdvectionSUPG

!syntax inputs /Kernels/ADPhaseFieldAdvectionSUPG

!syntax children /Kernels/ADPhaseFieldAdvectionSUPG



!bibtex bibliography

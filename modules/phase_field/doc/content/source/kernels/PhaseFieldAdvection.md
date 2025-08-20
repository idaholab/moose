# PhaseFieldAdvection

The phase field equation (known as the Cahn-Hilliard equation) is typically defined as below. As shown in this equation, the
`PhaseFieldAdvection` kernel implements the advection portion of the equation.

\begin{equation}
    \frac{\partial \phi}{\partial t} + \underbrace{\vec{v} \cdot \nabla \phi}_{\textrm{PhaseFieldAdvection}} = \gamma \nabla^2 G
\end{equation}
where $\phi$ is the phase field variable, $t$ is time, and $\vec{v}$ is a known velocity field that
advects the phase field variable. $\gamma$ is the mobility coefficient and $G$ is the chemical potential associated with interface between two phases.

The weak form of this portion of the equation is defined as: find $\phi_h$ such that:

\begin{equation}
(\Psi_i, \vec{v}\cdot\nabla \phi_h) = 0.
\end{equation}

## Example Syntax

This kernel is utilized from within the [`Kernels`](syntax/Kernels/index.md) block in conjunction
with a time derivative kernel ([TimeDerivative](/TimeDerivative.md)) and a body force kernel([BodyForce](/BodyForce.md)).

!listing modules/phase_field/test/tests/phase_field_advection/phase_field_mms.i block=Kernels

!syntax parameters /Kernels/PhaseFieldAdvection

!syntax inputs /Kernels/PhaseFieldAdvection

!syntax children /Kernels/PhaseFieldAdvection


# INSMomentumTractionForm

!syntax description /Kernels/INSMomentumTractionForm

This object adds the following terms for the Navier-Stokes momentum equation

\begin{equation}
\rho\left(\vec{u}\cdot\nabla\right)\vec{u} - \nabla\cdot\sigma - \vec{f}
\end{equation}

where $\rho$ is the density, $\vec{u}$ is the velocity, $\sigma$ is the total
stress tensor, and $\vec{f}$ is a body force per unit volume. The total stress
tensor is generally defined by

\begin{equation}
\label{traction}
\sigma = -p\bm{I} + \mu\left(\nabla\vec{u} + \left(\nabla\vec{u}\right)^T\right)
\end{equation}

where $p$ is the pressure, $\bm{I}$ is the identity matrix, and $\mu$ is the
dynamic viscosity. The second term on the RHS of [traction] is what we call the
"traction" form of the viscous stress. For incompressible fluids with a constant
viscosity, this term can be simplified as described in [INSMomentumLaplaceForm.md].

!syntax parameters /Kernels/INSMomentumTractionForm

!syntax inputs /Kernels/INSMomentumTractionForm

!syntax children /Kernels/INSMomentumTractionForm

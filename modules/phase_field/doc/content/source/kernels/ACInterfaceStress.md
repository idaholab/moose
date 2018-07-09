# ACInterfaceStress

!syntax description /Kernels/ACInterfaceStress

Implements the Allen-Cahn term for the $-L_i\nabla\frac{\partial E_{el}}{\partial \nabla \eta_i}$
elastic energy contribution for the interface stress, where
$E_{el}=\frac12\sigma_{int}:\epsilon$. Its weak form is

\begin{equation}
R=-\frac12L\left(\nabla \frac{\partial \sigma_{int}}{\partial\nabla\eta_i}:\epsilon, \psi_m \right),
\end{equation}

where $\eta_i$ the non-conserved non-linear order parameter variable the kernel is
acting on, $L_i$ (`mob_name`) is the scalar (isotropic) mobility associated with
the order parameter, and $\psi_m$ is the test function.

Integrating by parts and applying the divergence theorem yields

\begin{equation}
R=\frac12L\left[
\left(\frac{\partial \sigma_{int}}{\partial\nabla\eta_i}:\epsilon, \nabla\psi_m\right) - \left<\frac{\partial \sigma_{int}}{\partial\nabla\eta_i}:\epsilon\cdot\vec n,\psi_m\right>
\right].
\end{equation}

!syntax parameters /Kernels/ACInterfaceStress

!syntax inputs /Kernels/ACInterfaceStress

!syntax children /Kernels/ACInterfaceStress

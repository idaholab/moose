# Diffusion

## Description

The steady-state diffusion equation on a domain $\Omega$ is defined as
\begin{equation}
-\nabla \cdot \nabla u = 0 \in \Omega.
\end{equation}

The weak form of this equation, in inner-product notation, is given by:

\begin{equation}
R_i(u_h) = (\nabla \psi_i, \nabla u_h) = 0 \quad \forall  \psi_i,
\end{equation}
where $\psi_i$ are the test functions and $u_h \in \mathcal{S}^h$ is the finite
element solution of the weak formulation.

The Jacobian is defined as:

\begin{equation}
\frac{\partial R_i(u_h)}{\partial u_j} = (\nabla \phi_i, \nabla \psi_j).
\end{equation}

!syntax parameters /Kernels/Diffusion

!syntax inputs /Kernels/Diffusion

!syntax children /Kernels/Diffusion

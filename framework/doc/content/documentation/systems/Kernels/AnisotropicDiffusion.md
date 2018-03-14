# AnisotropicDiffusion

## Description

The `AnisotropicDiffusion` kernel implements anistropic diffusion term on a domain ($\Omega$) given in its strong form as

\begin{equation}
\nabla\cdot -\widetilde{k} \nabla u = 0 \in \Omega,
\end{equation}
where $\widetilde{k}$ is the anisotropic
diffusion coefficient. Diffusion is anistropic if the diffusion rate varies with
direction. The corresponding weak form, using inner-product notation, is given by

\begin{equation}
R_i(u_h) = \underbrace{(\nabla \psi_i, \widetilde{k} \nabla
u_h)}_{\textrm{AnisotropicDiffusion}} - \langle\psi_i, \widetilde{k} \nabla u_h
\cdot \vec{n}\rangle\quad \forall \phi_i,
\end{equation}
where the first term denotes the inner product over the domain volume, the latter term denotes the
outward diffusion flux over the volume boundary ($\Gamma$), $\psi_i$ are the test functions, and $u_h
\in \mathcal{S}^h$ is the finite element solution of the weak formulation. The `AnisotropicDiffusion`
kernel implements the first (volume) term.

For a constant diffusion coefficient, the Jacobian is given by
\begin{equation}
\frac{\partial R_i(u_h)}{\partial u_j} =
(\nabla \phi_j, \widetilde{k} \nabla u_h).
\end{equation}

## Example Syntax

The `AnisotropicDiffusion` kernel may be used in a variety of physical models, including steady-state
and time-dependent diffusion, advection-diffusion-reaction, etc. A kernel block demonstrating the
`AnistropicDiffusion` syntax in a steady-state anistropic diffusion problem can be found below:

!listing test/tests/kernels/anisotropic_diffusion/aniso_diffusion.i block=Kernels

!alert note
The anistropic diffusion coefficient $\widetilde{k}$ is a three-dimensional tensor supplied through a
string with nine space separated real values. The entries correspond to $xx$, $xy$, $xz$, $yx$, $yy$,
$yz$, $zx$, $zy$, and $zz$, respectively. Also, this problem is 2-dimensional so the corresponding
$zx$, $zy$, and $zz$ terms are zero.

!syntax parameters /Kernels/AnisotropicDiffusion

!syntax inputs /Kernels/AnisotropicDiffusion

!syntax children /Kernels/AnisotropicDiffusion

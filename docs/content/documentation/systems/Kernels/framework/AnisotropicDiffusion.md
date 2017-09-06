<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# AnisotropicDiffusion

## Description

This kernel implements anistropic diffusion given in its strong form as $$\nabla
\cdot -\widetilde{k} \nabla u$$ where $\widetilde{k}$ is the anisotropic
diffusion coefficient. Diffusion is anistropic if the diffusion rate varies with
direction. The corresponding weak form is given by $$(\nabla \psi_i,
\widetilde{k} \nabla u_h) \ - <\psi_i, \widetilde{k} \nabla u_h \cdot \vec{n}>$$
where the first term denotes the inner product over the domain volume ($\Omega$)
and the latter term denotes the outward diffusion flux over the volume's
boundary $\Gamma$. The `AnisotropicDiffusion` kernel implements the first/volume term.

For a constant diffusion coefficient, the Jacobian is given by $$(\nabla \phi_j,
\widetilde{k} \nabla u_h)$$

## Example Syntax

The `AnisotropicDiffusion` kernel may be used in a variety of physical models,
including steady-state and time-dependent diffusion,
advection-diffusion-reaction, etc. A kernel block demonstrating the
`AnistropicDiffusion` syntax in a steady-state anistropic
diffusion problem can be found below:

!listing test/tests/kernels/anisotropic_diffusion/aniso_diffusion.i
 block=Kernels label=False

 Note that the anistropic diffusion coefficient $\widetilde{k}$ is a
 three-dimensional tensor supplied through a string with nine space separated
 real values. The entries correspond to $xx\ xy\ xz\ yx\ yy\ yz\ zx\ zy\ zz$
 respectively. Note that this problem is 2-dimensional so the corresponding $zx\
 zy\ zz$ terms are zeroed.

!syntax parameters /Kernels/AnisotropicDiffusion

!syntax inputs /Kernels/AnisotropicDiffusion

!syntax children /Kernels/AnisotropicDiffusion

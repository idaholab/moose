# FVAnisotropicDiffusion

!syntax description /FVKernels/FVAnisotropicDiffusion

An anisotropic diffusion term that is discretized using the finite-volume method:

!equation
- \nabla \cdot \mathbf{k} \nabla \phi,

where $\mathbf{k}$ is the diagonal tensor diffusion coefficient and $\phi$
is the diffusing variable. The discretized form of the equation above
over en element is the following:

!equation
- \sum\limits_f \mathbf{k}_f (\nabla \phi)_f \mathbf{S}_f

where $\mathbf{S}_f$ denotes the surface vector of face $f$ of the element.
Furthermore, the face gradient, (\nabla \phi)_f, is determined using a
central difference scheme combined with non-orthogonal correction.
Lastly, components of the diffusion (diagonal) tensor can be
either interpolated to the face using a geometric arithmetic average:

!equation
(k_i)_f = g (k_i)_C + (1 - g) (k_i)_N,

or a harmonic average:

!equation
1 / (k_i)_f = g / (k_i)_C + (1 - g) / (k_i)_N,

where $g$ is the interpolation weight, and subscripts $C$ and $N$ represent
element and neighbor values.

!listing tests/fvkernels/fv_anisotropic_diffusion/fv_anisotropic_diffusion.i

!syntax parameters /FVKernels/FVAnisotropicDiffusion

!syntax inputs /FVKernels/FVAnisotropicDiffusion

!syntax children /FVKernels/FVAnisotropicDiffusion

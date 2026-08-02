# LinearFVAnisotropicDiffusionFunctorNeumannBC

## Description

`LinearFVAnisotropicDiffusionFunctorNeumannBC` prescribes the complete diffusive flux

!equation
g_b = \vec{n}_b \cdot \mathbb{D}_b \nabla u_b

for a linear finite volume variable $u$, where $\vec{n}_b$ is the outward boundary normal and
$\mathbb{D}_b$ is the diagonal tensor supplied through
[!param](/LinearFVBCs/LinearFVAnisotropicDiffusionFunctorNeumannBC/diffusion_tensor). The flux
$g_b$ is supplied by the [!param](/LinearFVBCs/LinearFVAnisotropicDiffusionFunctorNeumannBC/functor)
parameter.

The boundary condition reconstructs the normal gradient by decomposing the tensor-weighted flux:

!equation
g_b = a_n \frac{\partial u}{\partial n} +
\left(\mathbb{D}_b\vec{n}_b-a_n\vec{n}_b\right)\cdot\nabla u_b,

where $a_n=\vec{n}_b\cdot\mathbb{D}_b\vec{n}_b$. Therefore,

!equation
\frac{\partial u}{\partial n} = \frac{g_b-
\left(\mathbb{D}_b\vec{n}_b-a_n\vec{n}_b\right)\cdot\nabla u_b}{a_n}.

The bracketed vector is tangential to the boundary. Its contribution is used to reconstruct the
normal gradient and boundary value, but it is not added to the kernel residual because the
prescribed $g_b$ already contains the complete anisotropic flux. The same diffusion tensor functor
should be supplied to this boundary condition and to [LinearFVAnisotropicDiffusion.md].

!syntax parameters /LinearFVBCs/LinearFVAnisotropicDiffusionFunctorNeumannBC

!syntax inputs /LinearFVBCs/LinearFVAnisotropicDiffusionFunctorNeumannBC

!syntax children /LinearFVBCs/LinearFVAnisotropicDiffusionFunctorNeumannBC

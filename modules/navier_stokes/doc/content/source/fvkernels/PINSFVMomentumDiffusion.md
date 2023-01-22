# PINSFVMomentumDiffusion

This kernel implements the diffusion term of the porous media Navier Stokes momentum equation.
This diffusion term represents a Brinkman-type viscous stress.

The incompressible approximation simplifies the expression of the stress tensor and the
diffusion term is expressed in terms of the superficial velocity:

\begin{equation}
-\nabla \cdot \left( \mu \nabla \dfrac{\vec{u_d}}{\epsilon} \right) = -\nabla \cdot \left( \dfrac{\mu}{\epsilon} \nabla \vec{u_d} \right) -\nabla \cdot \left( \mu u_d \nabla \dfrac{1}{\epsilon} \right)
\end{equation}

The divergence theorem is used to compute this term by examining its flux through the element's faces.
The second term is challenging to compute near discontinuities in porosity and is not included by default.
For continuous porosity variations, the `smooth_porosity` parameter may be used to include it.

!syntax parameters /FVKernels/PINSFVMomentumDiffusion

!syntax inputs /FVKernels/PINSFVMomentumDiffusion

!syntax children /FVKernels/PINSFVMomentumDiffusion

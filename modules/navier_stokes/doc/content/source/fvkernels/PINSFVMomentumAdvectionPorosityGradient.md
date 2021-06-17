# PINSFVMomentumAdvectionPorosityGradient

This object implements the inverse porosity gradient term of the porous media Navier Stokes momentum
equation advection term, bolded below.

\begin{equation}
\nabla \cdot \left(\dfrac{1}{\epsilon} \rho\vec u_d \otimes \vec u_d\right) = \dfrac{1}{\epsilon} \nabla \cdot \left(\rho\vec u_d \otimes \vec u_d\right) + \mathbf{\left(\rho\vec u_d \otimes \vec u_d\right) \nabla \dfrac{1}{\epsilon}}
\end{equation}

This term will induce oscillations in the solution near porosity discontinuities.

!syntax parameters /FVKernels/PINSFVMomentumAdvectionPorosityGradient

!syntax inputs /FVKernels/PINSFVMomentumAdvectionPorosityGradient

!syntax children /FVKernels/PINSFVMomentumAdvectionPorosityGradient

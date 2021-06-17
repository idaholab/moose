# PINSFVMomentumAdvection

This object implements part of the $\nabla \cdot \left(\epsilon \rho\vec u \otimes \vec u\right)$ component
terms of the incompressible porous media Navier Stokes momentum equation. These terms are
expressed in terms of the superficial velocity $u_d$.

\begin{equation}
\nabla \cdot \left(\dfrac{1}{\epsilon} \rho\vec u_d \otimes \vec u_d\right) = \mathbf{\dfrac{1}{\epsilon} \nabla \cdot \left(\rho\vec u_d \otimes \vec u_d\right)} + \left(\rho\vec u_d \otimes \vec u_d\right) \nabla \dfrac{1}{\epsilon}
\end{equation}

This kernel only models the first term as a surface contribution using the divergence theorem.
The second term may be modeled in the absence of porosity discontinuities using the
[PINSFVMomentumAdvectionPorosityGradient.md] kernel.

An average or Rhie-Chow interpolation can be used for the advecting velocity interpolation.
When using the Rhie-Chow interpolation to compute the mass flow across faces, this kernel
reverts by default to average interpolation near porosity changes. This behavior may be disabled
with the `smooth_porosity` boolean.

An average or upwind interpolation can be used for the advected quantity, which in this
case is the momentum component $\rho u_{di}$ where $u_{i}$ denotes the x, y, or z
component of the intersitial velocity in Cartesian coordinates, or the r or z component of
the velocity in RZ coordinates.

!syntax parameters /FVKernels/PINSFVMomentumAdvection

!syntax inputs /FVKernels/PINSFVMomentumAdvection

!syntax children /FVKernels/PINSFVMomentumAdvection

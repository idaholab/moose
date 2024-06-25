# PINSFEFluidVelocityTimeDerivative

!syntax description /Kernels/PINSFEFluidVelocityTimeDerivative

If using the conservative form, the time derivative of the density is taken into account in the contribution to
the residual $R_{time}$. If not, only the time derivative of velocity is considered.

!equation
R_{time} = (\rho \dfrac{\partial v_i}{\partial t} + v_i \dfrac{\partial \rho}{\partial t})\psi

with $\rho$ the fluid density, $v_i$ the component of the (superficial) velocity specified
with the [!param](/Kernels/PINSFEFluidVelocityTimeDerivative/variable) parameter, and $\psi$
the test function.

!alert note
The porosity is assumed to be constant in time.

!syntax parameters /Kernels/PINSFEFluidVelocityTimeDerivative

!syntax inputs /Kernels/PINSFEFluidVelocityTimeDerivative

!syntax children /Kernels/PINSFEFluidVelocityTimeDerivative

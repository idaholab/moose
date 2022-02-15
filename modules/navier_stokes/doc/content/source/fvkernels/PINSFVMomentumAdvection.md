# PINSFVMomentumAdvection

This object implements the $\nabla \cdot \left(\epsilon \rho\vec u \otimes \vec
u\right) = \nabla \cdot \left(\dfrac{1}{\epsilon} \rho\vec u_d \otimes \vec u_d\right)$ component
terms of the incompressible porous media Navier Stokes momentum equation where
$u$ represents the free-flow velocity and $u_d$ represents the superficial
velocity. The nonlinear variables used in PINSFV correspond to the superficial velocity.

An average or Rhie-Chow interpolation can be used for the advecting velocity interpolation.
An average or upwind interpolation can be used for the advected quantity, which in this
case is the momentum component $\rho u_{di}$ where $u_{i}$ denotes the x, y, or z
component of the intersitial velocity in Cartesian coordinates, or the r or z component of
the velocity in RZ coordinates.

!syntax parameters /FVKernels/PINSFVMomentumAdvection

!syntax inputs /FVKernels/PINSFVMomentumAdvection

!syntax children /FVKernels/PINSFVMomentumAdvection

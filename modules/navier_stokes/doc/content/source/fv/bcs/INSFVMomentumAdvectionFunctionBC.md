# INSFVMomentumAdvectionFunctionBC

This object is only meant for use in MMS testing. It implements the
$\rho\left(\vec u \cdot\nabla\right)\vec u$ component
terms of the
incompressible Navier Stokes momentum equation. An average or Rhie-Chow
interpolation can be used for the advecting velocity interpolation. An average
or upwind interpolation can be used for the advected quantity, which in this
case is the momentum component $\rho u_i$ where $u_i$ denotes the x, y, or z
component of the velocity in Cartesian coordinates, or the r or z component of
the velocity in RZ coordinates. Second order convergence is preserved by using
exact solution information to construct ghost cell information.

!syntax parameters /FVBCs/INSFVMomentumAdvectionFunctionBC

!syntax inputs /FVBCs/INSFVMomentumAdvectionFunctionBC

!syntax children /FVBCs/INSFVMomentumAdvectionFunctionBC

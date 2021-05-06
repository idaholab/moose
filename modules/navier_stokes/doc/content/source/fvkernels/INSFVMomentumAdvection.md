# INSFVMomentumAdvection

This object implements the $\nabla \cdot \left(\rho\vec u \otimes \vec u\right)$ component
terms of the
incompressible Navier Stokes momentum equation. An average or Rhie-Chow
interpolation can be used for the advecting velocity interpolation. An average
or upwind interpolation can be used for the advected quantity, which in this
case is the momentum component $\rho u_i$ where $u_i$ denotes the x, y, or z
component of the velocity in Cartesian coordinates, or the r or z component of
the velocity in RZ coordinates.

!syntax parameters /FVKernels/INSFVMomentumAdvection

!syntax inputs /FVKernels/INSFVMomentumAdvection

!syntax children /FVKernels/INSFVMomentumAdvection

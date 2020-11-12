# INSFVMomentumPressureRZ

This object adds a $-p/r$ term to the radial part of the incompressible
Navier-Stokes equations (+make sure you apply this object to your radial
velocity variable+). This term falls out of the translation from
$\nabla p$ to $\nabla \cdot Ip$ in RZ coordinates. The $\nabla \cdot Ip$ term is
handled with the [INSFVMomentumPressure.md] object.

!syntax parameters /FVKernels/INSFVMomentumPressureRZ

!syntax inputs /FVKernels/INSFVMomentumPressureRZ

!syntax children /FVKernels/INSFVMomentumPressureRZ

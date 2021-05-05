# PINSFVMomentumPressure


This object adds the $\epsilon \nabla p$ term of the
incompressible Navier Stokes momentum equation as a volumetric term.
This formulation causes oscillation near porosity discontinuities and the
[PINSFVMomentumPressureFlux.md] kernel should be preferred then.

The pressure is a Lagrange Multiplier that ensures the incompressibility constraint.

!syntax parameters /FVKernels/PINSFVMomentumPressure

!syntax inputs /FVKernels/PINSFVMomentumPressure

!syntax children /FVKernels/PINSFVMomentumPressure

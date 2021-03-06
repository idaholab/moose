# PINSFVMomentumPressureFlux


This object adds the $\epsilon \nabla p$ pressure gradient term of the
incompressible porous media Navier Stokes momentum equation as a surface term using the divergence
theorem.
This formulation better handles discontinuities in porosity as it does not attempt to
compute the pressure gradient near the discontinuity. It is otherwise strictly equivalent
to its volumetric equivalent [PINSFVMomentumPressure.md].

The pressure is a Lagrange Multiplier that ensures the incompressibility constraint.

!syntax parameters /FVKernels/PINSFVMomentumPressureFlux

!syntax inputs /FVKernels/PINSFVMomentumPressureFlux

!syntax children /FVKernels/PINSFVMomentumPressureFlux

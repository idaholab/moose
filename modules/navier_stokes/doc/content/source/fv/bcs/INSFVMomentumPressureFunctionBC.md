# INSFVMomentumPressureFunctionBC

This object is only meant for use in MMS testing. It implements the $\nabla p$ term of the
incompressible Navier Stokes momentum equation along boundary faces, preserving
second order convergence by using the exact solution to construct ghost cell information.

!syntax parameters /FVBCs/INSFVMomentumPressureFunctionBC

!syntax inputs /FVBCs/INSFVMomentumPressureFunctionBC

!syntax children /FVBCs/INSFVMomentumPressureFunctionBC

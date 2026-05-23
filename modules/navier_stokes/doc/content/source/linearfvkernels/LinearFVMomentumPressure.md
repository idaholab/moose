# LinearFVMomentumPressure

This object adds the $-\nabla p$ term to the right hand side of
the Navier Stokes momentum equations.

By default, the gradient is the Green-Gauss gradient from the pressure variable. For linear SIMPLE
solves, [!param](/LinearFVKernels/LinearFVMomentumPressure/rhie_chow_user_object) may be supplied so
the kernel uses the pressure-gradient selection from [RhieChowMassFlux.md], including reconstructed
pressure gradients when enabled there.

!syntax parameters /LinearFVKernels/LinearFVMomentumPressure

!syntax inputs /LinearFVKernels/LinearFVMomentumPressure

!syntax children /LinearFVKernels/LinearFVMomentumPressure

# LinearFVMomentumPressure

This object adds the $-\nabla p$ term to the right hand side of
the Navier Stokes momentum equations.

The gradient comes from this kernel's
[!param](/LinearFVKernels/LinearFVMomentumPressure/gradient_method) parameter, or from the pressure
variable's default gradient method when that parameter is omitted. For linear SIMPLE solves,
[RhieChowMassFlux.md] can reuse this kernel's pressure gradient field through its
[!param](/UserObjects/RhieChowMassFlux/momentum_pressure_kernel) parameter so the momentum predictor
and H/A construction stay consistent.

!syntax parameters /LinearFVKernels/LinearFVMomentumPressure

!syntax inputs /LinearFVKernels/LinearFVMomentumPressure

!syntax children /LinearFVKernels/LinearFVMomentumPressure

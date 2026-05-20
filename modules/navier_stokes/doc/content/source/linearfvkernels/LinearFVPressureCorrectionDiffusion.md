# LinearFVPressureCorrectionDiffusion

!syntax description /LinearFVKernels/LinearFVPressureCorrectionDiffusion

This kernel adds the pressure correction diffusion term used by the linear finite volume
[SIMPLE.md] algorithm. The pressure correction equation uses `Ainv`, the inverse momentum
matrix diagonal, as the diffusion tensor. The `Ainv` functor is computed by
[RhieChowMassFlux.md].

For direct syntax, add the kernel to the `[LinearFVKernels]` block and pass the same kernel name
to the [!param](/UserObjects/RhieChowMassFlux/p_diffusion_kernel) parameter of
[RhieChowMassFlux.md]:

!listing modules/navier_stokes/test/tests/finite_volume/ins/lid-driven/linear-segregated/lid-driven-segregated.i block=UserObjects/rc

!listing modules/navier_stokes/test/tests/finite_volume/ins/lid-driven/linear-segregated/lid-driven-segregated.i block=LinearFVKernels/p_diffusion

## Selecting Interpolation

Select `average` or `harmonic` interpolation for the `Ainv` face values with
[!param](/UserObjects/RhieChowMassFlux/pressure_diffusion_interpolation) on
[RhieChowMassFlux.md]. When using the Physics syntax, set
[!param](/Physics/NavierStokes/FlowSegregated/WCNSLinearFVFlowPhysics/pressure_diffusion_interpolation)
instead:

!listing modules/navier_stokes/test/tests/finite_volume/ins/lid-driven/linear-segregated/lid-driven-segregated-physics.i line=pressure_diffusion_ainv_interp_method

!listing modules/navier_stokes/test/tests/finite_volume/ins/lid-driven/linear-segregated/lid-driven-segregated-physics.i start=momentum_advection_interpolation end=pressure_diffusion_interpolation include-end=true

!syntax parameters /LinearFVKernels/LinearFVPressureCorrectionDiffusion

!syntax inputs /LinearFVKernels/LinearFVPressureCorrectionDiffusion

!syntax children /LinearFVKernels/LinearFVPressureCorrectionDiffusion

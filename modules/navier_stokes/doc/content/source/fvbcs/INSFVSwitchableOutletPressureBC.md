# INSFVSwitchableOutletPressureBC

!syntax description /FVBCs/INSFVSwitchableOutletPressureBC

The file is similar to [INSFVOutletPressureBC.md] but it allows the boundary conditions to be switched on/off.

This object simply wraps [`FVFunctionDirichletBC`](FVFunctionDirichletBC.md), so
a required parameter is `function` describing the pressure along an outlet
boundary. The `variable` parameter should correspond to the pressure
variable. `INSFVSwitchableOutletPressureBC` also inherits from `INSFVFullyDevelopedBC`
which allows modifications to the coefficients used to compute the Rhie-Chow
interpolation in [`INSFVMomentumAdvection`](INSFVMomentumAdvection.md). When
applying a `INSFVSwitchableOutletPressureBC` for the pressure, no `FVBCs` should be given
for the velocity on the same `boundary`. In this way a zero viscous flux is
implicitly applied for the velocity on the `boundary`.

The switch works as follows:

- If `switch = true`: the boundary condition is applied as described above.

- If `switch = false`: the boundary condition is not applied and a single sided extrapolation to the boundary
  is applied from internal extrapolation. The user can expect second order convergence.

In both cases, the interpolated value at the face is contoled by `face_limiter`.
By default, `face_limiter = 1.0`.
`face_limiter` value is controllable during runtime.

!syntax parameters /FVBCs/INSFVSwitchableOutletPressureBC

!syntax inputs /FVBCs/INSFVSwitchableOutletPressureBC

!syntax children /FVBCs/INSFVSwitchableOutletPressureBC

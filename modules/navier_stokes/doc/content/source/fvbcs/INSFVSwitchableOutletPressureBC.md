# INSFVSwitchableOutletPressureBC

!syntax description /FVBCs/INSFVSwitchableOutletPressureBC

The file is similar to [INSFVOutletPressureBC.md] but it allows the boundary conditions to be switched on/off.

The `switch_bc` parameter is a boolean that is used to turn the boundary condition on/off.
The parameter is controllable during runtime and can be controlled via a `BoolFunctionControl`.

The switch works as follows:

- If `switch_bc = true`: the boundary condition is applied as described in [INSFVOutletPressureBC.md].

- If `switch_bc = false`: the boundary condition is not applied and a single sided extrapolation to the boundary
  is applied from internal extrapolation.

In both cases, the interpolated value at the face is controlled by `face_limiter`.
By default, `face_limiter = 1.0`.
`face_limiter` value is controllable during runtime.

!syntax parameters /FVBCs/INSFVSwitchableOutletPressureBC

!syntax inputs /FVBCs/INSFVSwitchableOutletPressureBC

!syntax children /FVBCs/INSFVSwitchableOutletPressureBC

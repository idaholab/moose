# INSFVOutletPressureBC

This object simply wraps [`FVFunctionDirichletBC`](FVFunctionDirichletBC.md), so
a required parameter is `function` describing the pressure along an outlet
boundary. The `variable` parameter should correspond to the pressure
variable. `INSFVOutletPressureBC` also inherits from `INSFVFullyDevelopedBC`
which allows modifications to the coefficients used to compute the Rhie-Chow
interpolation in [`INSFVMomentumAdvection`](INSFVMomentumAdvection.md). When
applying a `INSFVOutletPressureBC` for the pressure, no `FVBCs` should be given
for the velocity on the same `boundary`. In this way a zero viscous flux is
implicitly applied for the velocity on the `boundary`.

!syntax parameters /FVBCs/INSFVOutletPressureBC

!syntax inputs /FVBCs/INSFVOutletPressureBC

!syntax children /FVBCs/INSFVOutletPressureBC

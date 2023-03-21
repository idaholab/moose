# INSFVAveragePressureValueBC

This object wraps [FVBoundaryIntegralValueConstraint.md], meaning that this
object can be used to impose an average pressure value on a boundary. The
`variable` parameter should correspond to the pressure variable. When applying a
`INSFVAveragePressureValueBC` for the pressure, no `FVBCs` should be given for
the velocity on the same `boundary`. In this way a zero viscous flux is
implicitly applied for the velocity on the `boundary`.

!syntax parameters /FVBCs/INSFVAveragePressureValueBC

!syntax inputs /FVBCs/INSFVAveragePressureValueBC

!syntax children /FVBCs/INSFVAveragePressureValueBC

# INSFVSymmetryScalarBC

This object ensures that the scalar quantity flux
perpendicular to a symmetry boundary is zero. This is generally
already achieved by the velocity and mass symmetry boundary conditions, which
set the fluid normal velocity to zero, but may required in their absence.

If solving the fluid flow equations simultaneously, in addition to the
`INSFVSymmetryScalarBC`, an [INSFVSymmetryVelocityBC.md]
should be applied for every velocity component
and an [INSFVSymmetryPressureBC.md] should be applied on the pressure, on a symmetry boundary.

!syntax parameters /FVBCs/INSFVSymmetryScalarBC

!syntax inputs /FVBCs/INSFVSymmetryScalarBC

!syntax children /FVBCs/INSFVSymmetryScalarBC

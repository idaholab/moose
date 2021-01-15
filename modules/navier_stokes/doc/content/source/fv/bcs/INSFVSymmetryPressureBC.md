# INSFVSymmetryPressureBC

Though applied to the pressure, this object ensures that the velocity
perpendicular to a symmetry boundary is zero by setting the mass flow rate
across the symmetry boundary to zero. In addition to the
`INSFVSymmetryPressureBC`, a [`INSFVSymmetryVelocityBC`](INSFVSymmetryVelocityBC.md)
should be applied for every velocity component on a symmetry boundary.

!syntax parameters /FVBCs/INSFVSymmetryPressureBC

!syntax inputs /FVBCs/INSFVSymmetryPressureBC

!syntax children /FVBCs/INSFVSymmetryPressureBC

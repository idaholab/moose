# INSFVSymmetryScalarBC

Though not applied to the velocity, this object ensures that the velocity
perpendicular to a symmetry boundary is zero by setting the mass flow rate
across the symmetry boundary to zero. In addition to the
`INSFVSymmetryScalarBC`, a [`INSFVSymmetryVelocityBC`](INSFVSymmetryVelocityBC.md)
should be applied for every velocity component on a symmetry boundary.

!syntax parameters /FVBCs/INSFVSymmetryScalarBC

!syntax inputs /FVBCs/INSFVSymmetryScalarBC

!syntax children /FVBCs/INSFVSymmetryScalarBC

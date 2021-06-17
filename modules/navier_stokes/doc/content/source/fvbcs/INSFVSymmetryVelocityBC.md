# INSFVSymmetryVelocityBC

This object implements a symmetry boundary condition for the velocity. It
applies boundary forces such that the gradient of the velocity parallel to the
boundary is zero in the boundary normal direction. A `INSFVSymmetryVelocityBC`
should be applied for every velocity component on a symmetry boundary. Similarly
an [`INSFVSymmetryPressureBC`](INSFVSymmetryPressureBC.md) should be applied for
the pressure on the symmetry boundary.

!syntax parameters /FVBCs/INSFVSymmetryVelocityBC

!syntax inputs /FVBCs/INSFVSymmetryVelocityBC

!syntax children /FVBCs/INSFVSymmetryVelocityBC

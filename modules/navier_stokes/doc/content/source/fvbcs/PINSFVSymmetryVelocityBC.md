# PINSFVSymmetryVelocityBC

## Overview

This object implements a symmetry boundary condition for the superficial velocity. It
applies boundary forces such that the gradient of the velocity parallel to the
boundary is zero in the boundary normal direction. A `PINSFVSymmetryVelocityBC`
should be applied for every superficial velocity component on a symmetry boundary. Similarly
an [`INSFVSymmetryPressureBC`](INSFVSymmetryPressureBC.md) should be applied for
the pressure on the symmetry boundary.

!syntax description /FVBCs/PINSFVSymmetryVelocityBC

!syntax parameters /FVBCs/PINSFVSymmetryVelocityBC

!syntax inputs /FVBCs/PINSFVSymmetryVelocityBC

!syntax children /FVBCs/PINSFVSymmetryVelocityBC

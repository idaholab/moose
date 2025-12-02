# MFEMComplexVectorTangentialDirichletBC

!if! function=hasCapability('mfem')

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on the tangential
components of a complex-valued $H(\mathrm{curl})$ conforming vector FE at a boundary. The boundary value is
a coefficient that may vary in space and/or time.

!syntax parameters /BCs/MFEMComplexVectorTangentialDirichletBC

!syntax inputs /BCs/MFEMComplexVectorTangentialDirichletBC

!syntax children /BCs/MFEMComplexVectorTangentialDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md

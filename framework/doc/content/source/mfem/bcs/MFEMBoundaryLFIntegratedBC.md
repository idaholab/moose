# MFEMBoundaryLFIntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(f, v)_{\partial\Omega} \,\,\, \forall v \in V

where the test variable $v \in H^1$ and $f$ is a scalar coefficient. Often used for representing
Neumann-type boundary conditions.

!syntax parameters /BCs/MFEMBoundaryLFIntegratedBC

!syntax inputs /BCs/MFEMBoundaryLFIntegratedBC

!syntax children /BCs/MFEMBoundaryLFIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md

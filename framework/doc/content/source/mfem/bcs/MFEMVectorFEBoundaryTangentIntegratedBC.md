# MFEMVectorFEBoundaryTangentIntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorFEBoundaryTangentIntegratedBC

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(\vec n \times \vec f, \vec v)_{\partial\Omega} \,\,\, \forall v \in V

where $\vec v \in H(\mathrm{curl})$, $f$ is a scalar coefficient, and $\hat n$ is the
outward facing unit normal vector on the boundary.

## Example Input File Syntax

!listing test/tests/mfem/complex/complex_waveguide.i block=BCs

!syntax parameters /BCs/MFEMVectorFEBoundaryTangentIntegratedBC

!syntax inputs /BCs/MFEMVectorFEBoundaryTangentIntegratedBC

!syntax children /BCs/MFEMVectorFEBoundaryTangentIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md

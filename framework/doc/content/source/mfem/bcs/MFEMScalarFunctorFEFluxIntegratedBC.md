# MFEMScalarFunctorFEFluxIntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMScalarFunctorFEFluxIntegratedBC

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(f, \vec v \cdot \hat n)_{\partial\Omega} \,\,\, \forall v \in V

where $\vec v \in H(\mathrm{div})$, $f$ is a scalar coefficient, and $\hat n$ is the
outward facing unit normal vector on the boundary.

## Example Input File Syntax

!listing test/tests/mfem/kernels/darcy.i block=BCs

!syntax parameters /BCs/MFEMScalarFunctorFEFluxIntegratedBC

!syntax inputs /BCs/MFEMScalarFunctorFEFluxIntegratedBC

!syntax children /BCs/MFEMScalarFunctorFEFluxIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md

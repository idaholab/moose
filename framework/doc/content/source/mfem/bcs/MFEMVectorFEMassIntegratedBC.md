# MFEMVectorFEMassIntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorFEMassIntegratedBC

## Overview

Adds the boundary integrator for integrating the bilinear form

!equation
(\vec u, \vec v)_{\partial\Omega} \,\,\, \forall v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$.

## Example Input File Syntax

!listing test/tests/mfem/kernels/complexmouse.i block=BCs

!syntax parameters /BCs/MFEMVectorFEMassIntegratedBC

!syntax inputs /BCs/MFEMVectorFEMassIntegratedBC

!syntax children /BCs/MFEMVectorFEMassIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md

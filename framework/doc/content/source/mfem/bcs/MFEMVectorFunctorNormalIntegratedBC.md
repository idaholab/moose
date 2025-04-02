# MFEMVectorFunctorNormalIntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorFunctorNormalIntegratedBC

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(\vec f \cdot \hat n, v)_{\partial\Omega} \,\,\, \forall v \in V

where $v \in H^1$, $\vec f$ is a vector functor, and $\hat n$ is the outward facing unit normal
vector on the boundary.

!syntax parameters /BCs/MFEMVectorFunctorNormalIntegratedBC

!syntax inputs /BCs/MFEMVectorFunctorNormalIntegratedBC

!syntax children /BCs/MFEMVectorFunctorNormalIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md

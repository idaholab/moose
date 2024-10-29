# MFEMVectorNormalIntegratedBC

## Summary

!syntax description /BCs/MFEMVectorNormalIntegratedBC

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(\vec f \cdot \vec n, v)_{\partial\Omega} \,\,\, \forall v \in V

where $v \in H^1$ and $\vec n$ is the outward facing unit normal vector on the surface.

!syntax parameters /BCs/MFEMVectorNormalIntegratedBC

!syntax inputs /BCs/MFEMVectorNormalIntegratedBC

!syntax children /BCs/MFEMVectorNormalIntegratedBC

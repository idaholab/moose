# MFEMTimeDerivativeMassKernel

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/MFEMTimeDerivativeMassKernel

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k \dot{u}, v)_\Omega \,\,\, \forall v \in V

where $\dot{u}, v \in H^1$ and $k$ is a scalar coefficient.

This term arises from the weak form of the operator

!equation
k \dot{u}

## Example Input File Syntax

!listing mfem/kernels/heatconduction.i

!syntax parameters /Kernels/MFEMTimeDerivativeMassKernel

!syntax inputs /Kernels/MFEMTimeDerivativeMassKernel

!syntax children /Kernels/MFEMTimeDerivativeMassKernel

!if-end!

!else
!include mfem/mfem_warning.md

# MFEMTimeDerivativeVectorFEMassKernel

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/MFEMTimeDerivativeVectorFEMassKernel

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k \dot{\vec u}, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\dot{\vec u}, \vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$ and $k$ is a scalar
coefficient.

This term arises from the weak form of the operator

!equation
k \dot{\vec u}

## Example Input File Syntax

!listing mfem/submeshes/hphi_magnetodynamic.i block=/Kernels

!syntax parameters /Kernels/MFEMTimeDerivativeVectorFEMassKernel

!syntax inputs /Kernels/MFEMTimeDerivativeVectorFEMassKernel

!syntax children /Kernels/MFEMTimeDerivativeVectorFEMassKernel

!if-end!

!else
!include mfem/mfem_warning.md

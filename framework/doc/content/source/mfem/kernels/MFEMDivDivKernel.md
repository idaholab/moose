# MFEMDivDivKernel

## Summary

!syntax description /Kernels/MFEMDivDivKernel

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k\vec\nabla \cdot \vec u, \vec\nabla \cdot \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{div})$ and $k$ is a scalar coefficient.

This term arises from the weak form of the grad div operator

!equation
-\vec\nabla \left(k \vec\nabla \cdot \vec u\right)

## Example Input File Syntax

!listing mfem/kernels/graddiv.i

!syntax parameters /Kernels/MFEMDivDivKernel

!syntax inputs /Kernels/MFEMDivDivKernel

!syntax children /Kernels/MFEMDivDivKernel

# MFEMConvectiveHeatFluxBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMConvectiveHeatFluxBC

## Overview

Adds the boundary integrator for integrating the bilinear form

!equation
\lambda(u-u_0, v)_{\partial\Omega} \,\,\, \forall v \in V

where $u, v \in H^1$ are the trial and test variables, and $\lambda, u_0$ are scalar coefficients on the boundary independent of $u$.

This boundary condition is particularly useful for thermal problems, where it can be used to
represent a heat transfer boundary condition

!equation
h(T-T_{\infty}, T')_{\partial\Omega} \,\,\, \forall T' \in V

by identifying $\lambda=h$ as the heat transfer coefficient on the boundary, $u=T$ as the trial variable
for the temperature, $v=T'$ as the test variable, and $u_0=T_{\infty}$ as the equilibrium temperature far from the boundary.

## Example Input File Syntax

!listing test/tests/mfem/kernels/heattransfer.i block=BCs

!syntax parameters /BCs/MFEMConvectiveHeatFluxBC

!syntax inputs /BCs/MFEMConvectiveHeatFluxBC

!syntax children /BCs/MFEMConvectiveHeatFluxBC

!if-end!

!else
!include mfem/mfem_warning.md

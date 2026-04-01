# MFEMNLConvectiveHeatFluxBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary integrator for integrating the non-linear form

!equation
(k(u)(u-u_0), v)_{\partial\Omega} \,\,\, \forall v \in V

where $u, v \in H^1$ are the trial and test variables, and $k(u), u_0$ are scalar coefficients on
the boundary. $k(u)$ may depend on the trial variable $u$.

This boundary condition is particularly useful for thermal problems, where it can be used to
represent a temperature-dependent heat transfer boundary condition

!equation
h(T) (T-T_{\infty}, T')_{\partial\Omega} \,\,\, \forall T' \in V

by identifying $k(u)=h(T)$ as the heat transfer coefficient on the boundary, $u=T$ as the trial variable
for the temperature, $v=T'$ as the test variable, and $u_0=T_{\infty}$ as the equilibrium temperature far from the boundary.

## Example Input File Syntax

!listing test/tests/mfem/kernels/nl_heattransfer.i block=BCs

!syntax parameters /BCs/MFEMNLConvectiveHeatFluxBC

!syntax inputs /BCs/MFEMNLConvectiveHeatFluxBC

!syntax children /BCs/MFEMNLConvectiveHeatFluxBC

!if-end!

!else
!include mfem/mfem_warning.md

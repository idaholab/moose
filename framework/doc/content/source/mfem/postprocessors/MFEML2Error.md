# MFEML2Error

!if! function=hasCapability('mfem')

## Summary

!syntax description /Postprocessors/MFEML2Error

## Overview

Postprocessor for calculating the L2 error of a scalar $H^1$ or $L^2$
conforming source variable compared to a scalar function.

!equation
\left\Vert u_{ex} - u_{h}\right\Vert_{\mathrm{L2}}

where $u_{h} \in H^1 \lor L^2$ and $u_{ex}$ is a scalar function.

## Example Input File Syntax

!listing mfem/kernels/irrotational.i

!syntax parameters /Postprocessors/MFEML2Error

!syntax inputs /Postprocessors/MFEML2Error

!syntax children /Postprocessors/MFEML2Error

!if-end!

!else
!include mfem/mfem_warning.md

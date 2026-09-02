# MFEMComplexL2Error

!if! function=hasCapability('mfem')

## Overview

Postprocessor for calculating the L2 error of a complex scalar $H^1$ or $L^2$
conforming source variable compared to a complex scalar function.

!equation
\left\Vert u_{ex} - u_{h}\right\Vert_{\mathrm{L2}}

where $u_{h} \in H^1 \lor L^2$ and $u_{ex}$ is a complex scalar function.

## Example Input File Syntax

!listing mfem/complex/mixed_sesquilinear.i block=Postprocessors

!syntax parameters /Postprocessors/MFEMComplexL2Error

!syntax inputs /Postprocessors/MFEMComplexL2Error

!syntax children /Postprocessors/MFEMComplexL2Error

!if-end!

!else
!include mfem/mfem_warning.md

# MFEMComplexVectorL2Error

!if! function=hasCapability('mfem')

## Overview

Postprocessor for calculating the L2 error of a complex vector $H^{1,d}$,
$L^{2,d}$, $H(\mathrm{curl})$, or $H(\mathrm{div})$  conforming source variable
compared to a complex vector function.

!equation
\left\Vert \vec{u}_{ex} - \vec{u}_{h}\right\Vert_{\mathrm{L2}}

where $\vec{u}_{h} \in H^{1,d} \lor L^{2,d} \lor H(\mathrm{curl}) \lor
H(\mathrm{div})$ and $\vec{u}_{ex}$ is a vector function.

## Syntax

!syntax parameters /Postprocessors/MFEMComplexVectorL2Error

!syntax inputs /Postprocessors/MFEMComplexVectorL2Error

!syntax children /Postprocessors/MFEMComplexVectorL2Error

!if-end!

!else
!include mfem/mfem_warning.md

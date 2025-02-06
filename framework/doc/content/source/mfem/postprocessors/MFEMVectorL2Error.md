# MFEMVectorL2Error

## Summary

!syntax description /Postprocessors/MFEMVectorL2Error

## Overview

Postprocessor for calculating the L2 error of a vector $H^{1,d}$,
$L^{2,d}$, $H(\mathrm{curl})$, or $H(\mathrm{div})$  conforming source variable
compared to a vector function.

!equation
\left\Vert \vec{u}_{ex} - \vec{u}_{h}\right\Vert_{\mathrm{L2}}

where $\vec{u}_{h} \in H^{1,d} \lor L^{2,d} \lor H(\mathrm{curl}) \lor
H(\mathrm{div})$ and $\vec{u}_{ex}$ is a vector function.

## Example Input File Syntax

!listing kernels/irrotational.i

!syntax parameters /Postprocessors/MFEMVectorL2Error

!syntax inputs /Postprocessors/MFEMVectorL2Error

!syntax children /Postprocessors/MFEMVectorL2Error

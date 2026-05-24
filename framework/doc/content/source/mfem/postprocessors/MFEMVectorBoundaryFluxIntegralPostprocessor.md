# MFEMVectorBoundaryFluxIntegralPostprocessor

!if! function=hasCapability('mfem')

## Overview

Postprocessor for calculating the integral of the flux of a vector $H^{1,d}$,
$L^{2,d}$, $H(\mathrm{curl})$, or $H(\mathrm{div})$  conforming source variable through a given boundary surface.

!equation
(k \vec v \cdot \hat n)_{\partial\Omega}

where $\vec v \in H^{1,d}$, $L^{2,d}$, $H(\mathrm{curl})$, or $H(\mathrm{div})$,
$k$ is an optional scalar coefficient, $\partial\Omega$ is the user-specified mesh boundary,
and $\hat n$ is the outward facing unit normal vector on the boundary. 
If $k$ is provided as a material coefficient, it must be defined on the boundary itself, rather than on a submesh adjacent to the boundary. 

## Example Input File Syntax

!listing mfem/submeshes/av_magnetostatic.i block=Postprocessors/CoilCurrent

!syntax parameters /Postprocessors/MFEMVectorBoundaryFluxIntegralPostprocessor

!syntax inputs /Postprocessors/MFEMVectorBoundaryFluxIntegralPostprocessor

!syntax children /Postprocessors/MFEMVectorBoundaryFluxIntegralPostprocessor

!if-end!

!else
!include mfem/mfem_warning.md
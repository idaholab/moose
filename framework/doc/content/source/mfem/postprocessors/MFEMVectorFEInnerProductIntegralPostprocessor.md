# MFEMVectorFEInnerProductIntegralPostprocessor

!if! function=hasCapability('mfem')

## Summary

!syntax description /Postprocessors/MFEMVectorFEInnerProductIntegralPostprocessor

## Overview

Postprocessor for calculating the integral of the dot product between two vector variables over a
user-specified subdomain of the mesh:

!equation
(k \vec u, \vec v)_\Omega

where $\vec u$, $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $k$ is an optional scalar coefficient,
and $\Omega$ is the user-specified mesh subdomain.

## Example Input File Syntax

!listing mfem/submeshes/cut_magnetostatic.i block=Postprocessors

!syntax parameters /Postprocessors/MFEMVectorFEInnerProductIntegralPostprocessor

!syntax inputs /Postprocessors/MFEMVectorFEInnerProductIntegralPostprocessor

!syntax children /Postprocessors/MFEMVectorFEInnerProductIntegralPostprocessor

!if-end!

!else
!include mfem/mfem_warning.md

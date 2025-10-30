# MFEMVectorL2InnerProductIntegralPostprocessor

!if! function=hasCapability('mfem')

## Summary

!syntax description /Postprocessors/MFEMVectorL2InnerProductIntegralPostprocessor

## Overview

Postprocessor for calculating the integral of the dot product between two L2 vector variables over a
user-specified subdomain of the mesh:

!equation
(k \vec u, \vec v)_\Omega

where $\vec u$, $\vec v \in \mathrm{L2}$, $k$ is an optional scalar coefficient,
and $\Omega$ is the user-specified mesh subdomain.

## Example Input File Syntax

!listing mfem/submeshes/av_magnetostatic.i block=Postprocessors

!syntax parameters /Postprocessors/MFEMVectorL2InnerProductIntegralPostprocessor

!syntax inputs /Postprocessors/MFEMVectorL2InnerProductIntegralPostprocessor

!syntax children /Postprocessors/MFEMVectorL2InnerProductIntegralPostprocessor

!if-end!

!else
!include mfem/mfem_warning.md

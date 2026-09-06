# MFEMElementAverageValue

!if! function=hasCapability('mfem')

## Overview

Computes the volumetric average of a scalar MFEM variable over the mesh or a block-restricted
subset of subdomains:

!equation
\bar{u} = \frac{\int_\Omega u \, \mathrm{d}V}{\int_\Omega \mathrm{d}V}

where $\Omega$ is the full mesh domain, or the union of subdomains specified by
[!param](/Postprocessors/MFEMElementAverageValue/block).

## Example Input File Syntax

!listing test/tests/mfem/meshgenerators/generated/test.i block=Postprocessors

!syntax parameters /Postprocessors/MFEMElementAverageValue

!syntax inputs /Postprocessors/MFEMElementAverageValue

!syntax children /Postprocessors/MFEMElementAverageValue

!if-end!

!else
!include mfem/mfem_warning.md

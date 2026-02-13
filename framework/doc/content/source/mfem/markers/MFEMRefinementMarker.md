# MFEMRefinementMarker

!if! function=hasCapability('mfem')

## Summary

Wrapper around `mfem::ThresholdRefiner`. Input file needs to have the name of an estimator to construct correctly.

## Overview

Currently contains one method for h-refinement (`hRefine`), which refines the mesh, and another for p-refinement (`pRefineMarker`), which marks what elements to refine.
Both advance an internal counter which determines how many refinement steps have been taken.

## Example Input File Syntax

!listing mfem/kernels/diffusion_amr.i block=Adaptivity

!syntax parameters /Adaptivity/Markers/MFEMRefinementMarker

!syntax inputs /Adaptivity/Markers/MFEMRefinementMarker

!syntax children /Adaptivity/Markers/MFEMRefinementMarker
!if-end!

!else
!include mfem/mfem_warning.md

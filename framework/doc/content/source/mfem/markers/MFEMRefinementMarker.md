# MFEMRefinementMarker

!if! function=hasCapability('mfem')

## Summary

Wrapper around `mfem::ThresholdRefiner`. Input file needs to have the name of an estimator to construct correctly.


## Overview

Currently contains one method for h-refinement (`HRefine`) and another for p-refinement (`MarkWithoutRefining`).
Both advance an internal counter which determines how many refinement steps have been taken.


!if-end!

!else
!include mfem/mfem_warning.md

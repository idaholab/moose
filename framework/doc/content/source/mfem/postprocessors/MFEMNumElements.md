# MFEMNumElements

!if! function=hasCapability('mfem')

## Overview

Postprocessor for the number of elements of the MFEM mesh, summed over all processes. It follows
MFEM adaptive mesh refinement, which changes the MFEM mesh but not the mesh that
[NumElements](NumElements.md) counts.

## Example Input File Syntax

In this example, the number of elements is computed after the mesh of a 2D definite Maxwell problem
has been refined adaptively.

!listing test/tests/mfem/postprocessors/curlcurl_amr_errors.i block=Postprocessors/num_elements

!syntax parameters /Postprocessors/MFEMNumElements

!syntax inputs /Postprocessors/MFEMNumElements

!syntax children /Postprocessors/MFEMNumElements

!if-end!

!else
!include mfem/mfem_warning.md

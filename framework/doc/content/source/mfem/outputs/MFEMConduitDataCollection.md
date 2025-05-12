# MFEMConduitDataCollection

!if! function=hasCapability('mfem')

## Summary

!syntax description /Outputs/MFEMConduitDataCollection

## Overview

`MFEMDataCollection` controlling output of data to an `mfem::ConduitDataCollection` for visualisation
in VisIt. Conduit output is typically significantly faster than other data collections since VisIt uses
the MFEM FE types for visualisation, and ParaViewDataCollection oversamples the degrees of freedom.

The user may choose the output protocol out of the following options: `hdf5`, `json`, `conduit_json`, and `conduit_bin`.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=Outputs/ConduitDataCollection

!syntax parameters /Outputs/MFEMConduitDataCollection

!syntax inputs /Outputs/MFEMConduitDataCollection

!syntax children /Outputs/MFEMConduitDataCollection

!if-end!

!else
!include mfem/mfem_warning.md

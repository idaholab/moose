# MFEMParaViewDataCollection

## Summary

!syntax description /Outputs/MFEMParaViewDataCollection

## Overview

`MFEMDataCollection` controlling output of data to an `mfem::ParaViewDataCollection` for
visualisation in ParaView. The VTK output format can be chosen (ASCII, BINARY, or BINARY32) as well
as whether to output the data on high-order elements or on a refined set of output points.

Many of Platypus's tests use ASCII MFEMParaViewDataCollection outputs for portability of the
reference files on different systems; however, for speed, it is recommended to prefer use of the
BINARY VTK format.

## Example Input File Syntax

!listing test/tests/kernels/diffusion.i block=Outputs

!syntax parameters /Outputs/MFEMParaViewDataCollection

!syntax inputs /Outputs/MFEMParaViewDataCollection

!syntax children /Outputs/MFEMParaViewDataCollection

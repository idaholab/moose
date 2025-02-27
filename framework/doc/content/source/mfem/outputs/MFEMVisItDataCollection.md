# MFEMVisItDataCollection

## Summary

!syntax description /Outputs/MFEMVisItDataCollection

## Overview

`MFEMDataCollection` controlling output of data to an `mfem::VisItDataCollection` for visualisation
in VisIt. The user may choose whether to output on a refined set of output points if oversampling of
output fields is desired.

## Example Input File Syntax

!listing test/tests/mfem/outputs/datacollections.i block=Outputs/VisItDataCollection

!syntax parameters /Outputs/MFEMVisItDataCollection

!syntax inputs /Outputs/MFEMVisItDataCollection

!syntax children /Outputs/MFEMVisItDataCollection

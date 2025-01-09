# MFEMFESpace

## Summary

!syntax description /FESpaces/MFEMFESpace

## Overview

Defines a finite element space for MFEM which one or more `MFEMVariable`s can be defined with respect to.

The family of shape functions is selected from the `fec_type` parameter, and the order is controlled
using the `fec_order` parameter.

## Example Input File Syntax

!listing test/tests/kernels/diffusion.i block=FESpaces Variables

!syntax parameters /FESpaces/MFEMFESpace

!syntax inputs /FESpaces/MFEMFESpace

!syntax children /FESpaces/MFEMFESpace

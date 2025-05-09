# MFEMHypreADS

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMHypreADS

## Overview

Defines and builds an `mfem::HypreADS` solver to use as a preconditioner or solver to solve the
MFEM equation system. Most effective for preconditioning and solving a grad-divergence problem when using
Raviart-Thomas elements, in which case the $H(\mathrm{div})$ FE space should be passed to the
`mfem::HypreADS` solver during construction.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter 
`low_order_refined` to `true`. Using an LOR solver improves performance for high polynomial 
order systems.

## Example Input File Syntax

!listing test/tests/mfem/kernels/graddiv.i block=FESpace Preconditioner Solver

!syntax parameters /Solver/MFEMHypreADS

!syntax inputs /Solver/MFEMHypreADS

!syntax children /Solver/MFEMHypreADS

!if-end!

!else
!include mfem/mfem_warning.md

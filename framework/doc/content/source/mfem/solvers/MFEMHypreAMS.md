# MFEMHypreAMS

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMHypreAMS

## Overview

Defines and builds an `mfem::HypreAMS` solver to use as a preconditioner or solver to solve the
MFEM equation system. Most effective for preconditioning and solving a curl-curl problem when using
Nédélec elements, in which case the $H(\mathrm{curl})$ FE space should be passed to the
`mfem::HypreAMS` solver during construction.

If the system of equations is singular - commonly arising, for example, when solving for the
magnetic vector potential in magnetostatic systems in the steady state - users should set the
`singular` parameter to `true` to add a small mass term to ensure solvability.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter 
[!param](Solvers/MFEMHypreAMS/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial 
order systems.

## Example Input File Syntax

!listing test/tests/mfem/kernels/curlcurl.i block=FESpace Preconditioner Solver

!syntax parameters /Solver/MFEMHypreAMS

!syntax inputs /Solver/MFEMHypreAMS

!syntax children /Solver/MFEMHypreAMS

!if-end!

!else
!include mfem/mfem_warning.md

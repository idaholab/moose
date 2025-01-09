# MFEMHypreBoomerAMG

## Summary

!syntax description /Solver/MFEMHypreBoomerAMG

## Overview

Defines and builds an `mfem::HypreBoomerAMG` solver to use as a preconditioner or solver to solve the MFEM equation system.

## Example Input File Syntax

!listing test/tests/kernels/diffusion.i block=Preconditioner Solver

!syntax parameters /Solver/MFEMHypreBoomerAMG

!syntax inputs /Solver/MFEMHypreBoomerAMG

!syntax children /Solver/MFEMHypreBoomerAMG

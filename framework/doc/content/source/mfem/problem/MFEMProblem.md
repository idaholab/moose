# MFEMProblem

## Summary

!syntax description /Problem/MFEMProblem

## Overview

`MFEMProblem` is derived from MOOSE's `ExternalProblem` Problem type, customised to set up the
 finite element problem using the MFEM FE library instead of MOOSE's default libMesh. Use of MFEM
 allows problem assembly and solution on GPU architectures as well as on CPU; desired device can be
 controlled by the [`MFEMExecutioner`](MFEMExecutioner.md) used to solve the problem.

`MFEMProblem` methods are called by `Actions` during parsing of the user's input file, which add
 and/or initialize members of the owned [MFEMProblemData](source/problem/MFEMProblemData.md) struct.
The order in which these actions are executed respects the dependencies declared in
 [PlatypusApp](source/base/PlatypusApp.md).

## Example Input File Syntax

In order to build the FE problem using the MFEM library on the backend, the `MFEMProblem` type must
 be used in `Problem` block in the user input.

!listing test/tests/kernels/diffusion.i block=Problem

!syntax parameters /Problem/MFEMProblem

!syntax inputs /Problem/MFEMProblem

!syntax children /Problem/MFEMProblem

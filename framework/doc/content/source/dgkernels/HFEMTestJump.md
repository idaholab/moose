# HFEMTestJump

!syntax description /DGKernels/HFEMTestJump

## Overview

This kernel adds a residual with a side-discontinuous variable
"lambda" weighted by inter-element jumps in test functions
corresponding to a discontinuous variable "u":
$\left( [ u^\ast ], \lambda \right)_\Gamma$

This is useful in diffusion equations with a hybrid finite element
method.  Refer to [DGKernels/index.md] for the full HFEM weak form for
Poisson's equation.

!syntax parameters /DGKernels/HFEMTestJump

!syntax inputs /DGKernels/HFEMTestJump

!syntax children /DGKernels/HFEMTestJump

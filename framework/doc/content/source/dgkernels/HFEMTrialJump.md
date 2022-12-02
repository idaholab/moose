# HFEMTrialJump

!syntax description /DGKernels/HFEMTrialJump

## Overview

This kernel adds a residual with inter-element jumps in a
discontinuous variable "u" weighted by test functions corresponding to
a side-discontinuous variable "lambda":
$\left( \lambda^\ast, [ u ] \right)_\Gamma$

This is useful in diffusion equations with a hybrid finite element
method.  Refer to [DGKernels/index.md] for the full HFEM weak form for
Poisson's equation.

!syntax parameters /DGKernels/HFEMTrialJump

!syntax inputs /DGKernels/HFEMTrialJump

!syntax children /DGKernels/HFEMTrialJump

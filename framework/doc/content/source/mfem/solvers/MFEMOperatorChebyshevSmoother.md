# MFEMOperatorChebyshevSmoother

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::OperatorChebyshevSmoother` solver. Chebyshev smoothing applies a
polynomial chosen to reduce error components over a target spectral interval of the diagonally
scaled operator `D^{-1}A`. The smoother estimates the maximum eigenvalue of `D^{-1}A` automatically
so that this interval can be selected from the operator supplied at setup time.

This object is a good candidate to be used as a smoother inside [MFEMGeometricMultigridSolver.md].

!syntax parameters /Solvers/MFEMOperatorChebyshevSmoother

!syntax inputs /Solvers/MFEMOperatorChebyshevSmoother

!syntax children /Solvers/MFEMOperatorChebyshevSmoother

!if-end!

!else
!include mfem/mfem_warning.md

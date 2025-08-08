# MFEMComplexKernel

!if! function=hasCapability('mfem')

## Summary

Base class for complex MFEM kernels applied to the weak form being solved.

## Overview

Complex MFEM kernels are responsible for providing complex-valued domain integrators to add to the weak form of the FE
problem accumulated in [EquationSystem](source/mfem/equation_systems/EquationSystem.md), along with any
required marker arrays to restrict the integrator(s) to subdomains. The `MFEMComplexKernel` is a container
for two objects of [MFEMKernel](source/mfem/kernels/MFEMKernel.md) type, one representing a real contribution to
the equation system, and the other representing an imaginary contribution. The real and imaginary kernels need not be the same,
but they do need to be applied to the same variable, which must be of [MFEMComplexVariable](source/mfem/variables/MFEMComplexVariable.md) type.

The integrators are applied to the weak form equation that is labeled according to the test variable
name of the kernel returned from `getTestVariableName()`. In the case of bilinear (or nonlinear)
forms, the trial variable that the integrator acts on is the variable returned from
`getTrialVariableName()`.

## Example Input File Syntax

The real and imaginary contributions to the `MFEMComplexKernel` can be set up by using the sub-blocks 
`real_part` and `imag_part` on the script.

!listing mfem/kernels/complex.i block=/Kernels

Note that the variable to which the kernel is applied must be complex-valued:

!listing mfem/kernels/complex.i block=/Variables

!syntax parameters /Kernels/MFEMComplexKernel

!syntax inputs /Kernels/MFEMComplexKernel

!syntax children /Kernels/MFEMComplexKernel


!if-end!

!else
!include mfem/mfem_warning.md

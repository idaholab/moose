# MFEMMixedSesquilinearFormKernel

!if! function=hasCapability('mfem')

## Summary

Base class for complex mixed MFEM kernels applied to the weak form being solved.

## Overview

Complex MFEM kernels are responsible for providing complex-valued domain integrators to add to the weak form of the FE
problem accumulated in [EquationSystem](source/mfem/equation_systems/EquationSystem.md), along with any
required marker arrays to restrict the integrator(s) to subdomains. The `MFEMMixedSesquilinearFormKernel` is a container
for two rectangular objects of [MFEMKernel](source/mfem/kernels/MFEMKernel.md) type, one representing a real contribution to
the equation system, and the other representing an imaginary contribution. Both kernels must be mixed kernels/ The real and imaginary kernels need not be the same,
but they do need to be applied to the same variable, which must be of [MFEMComplexVariable](source/mfem/variables/MFEMComplexVariable.md) type.

The integrators are applied to the weak form equation that is labeled according to the test variable
name of the kernel returned from `getTestVariableName()`. In the case of bilinear (or nonlinear)
forms, the trial variable that the integrator acts on is the variable returned from
`getTrialVariableName()`.

## Example Input File Syntax

The real and imaginary contributions to the `MFEMMixedSesquilinearFormKernel` can be set up by using the sub-blocks
`RealComponent` and `ImagComponent` on the script.

!listing mfem/complex/mixed_sesquilinear.i block=/Kernels

Note that the variable to which the kernel is applied must be complex-valued:

!listing mfem/complex/mixed_sesquilinear.i block=/Variables

!syntax parameters /Kernels/MFEMMixedSesquilinearFormKernel

!syntax inputs /Kernels/MFEMMixedSesquilinearFormKernel

!syntax children /Kernels/MFEMMixedSesquilinearFormKernel


!if-end!

!else
!include mfem/mfem_warning.md

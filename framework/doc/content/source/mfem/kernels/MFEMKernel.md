# MFEMKernel

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM kernels applied to the weak form being solved.

## Overview

MFEM kernels are responsible for providing domain integrators (inheriting from
`mfem::BilinearFormIntegrator` or `mfem::LinearFormIntegrator` to add to the weak form of the FE
problem accumulated in [EquationSystem](source/mfem/equation_systems/EquationSystem.md), along with any
required marker arrays to restrict the integrator(s) to subdomains.

The integrator is applied to the weak form equation that is labeled according to the test variable
name of the kernel returned from `getTestVariableName()`. In the case of bilinear (or nonlinear)
forms, the trial variable that the integrator acts on is the variable returned from
`getTrialVariableName()`. For consistency with MOOSE, and for the ease of associating weak form
equations (labeled by test variable) with the trial variable solved using them, the set of test
variable names is the same as the set of trial variable names for a square system.

`MFEMKernel` is a purely virtual base class. Derived classes should override the `createBFIntegrator`
and/or the `createLFIntegrator` methods to return a `BilinearFormIntegrator` and/or a
`LinearFormIntegrator` (respectively) to add to the `EquationSystem`.

!if-end!

!else
!include mfem/mfem_warning.md

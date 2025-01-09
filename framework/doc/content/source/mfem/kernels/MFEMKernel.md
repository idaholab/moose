# MFEMKernel

## Summary

Base class for MFEM kernels applied to the weak form being solved.

## Overview

MFEM kernels are responsible for providing domain integrators (inheriting from
`mfem::BilinearFormIntegrator` or `mfem::LinearFormIntegrator` to add to the weak form of the FE
problem accumulated in [EquationSystem](source/equation_systems/equation_system.md), along with any
required marker arrays to restrict the integrator(s) to subdomains.

The integrator is applied to the weak form equation that is labeled according to the test variable
name of the kernel returned from `getTestVariableName()`. In the case of bilinear (or nonlinear)
forms, the trial variable that the integrator acts on is the variable returned from
`getTrialVariableName()`. For consistency with MOOSE, and for the ease of associating weak form
equations (labeled by test variable) with the trial variable solved using them, the set of test
variable names is the same as the set of trial variable names for a square system.

`MFEMKernel` is a purely virtual base class. Derived classes should override the `createIntegrator`
 method to return an integrator to add to the `EquationSystem`.  

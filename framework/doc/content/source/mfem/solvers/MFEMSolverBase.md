# MFEMSolverBase

!if! function=hasCapability('mfem')

## Summary

Base class for `mfem::Solver` objects to use in MFEM problems.

## Overview

Classes derived from `MFEMSolverBase` can usually be used as preconditioners or linear solvers; the
`constructSolver` method should be overridden to construct a `shared_ptr` to an `mfem::Solver`
derived object, and the `getSolver` method should return the `shared_ptr` for use during a solve.

Problem-specific information - such as finite element spaces used in the set-up of some
preconditioners - can be passed to the `mfem::Solver` at construction time.

Most solvers have the option of being used as Low-Order-Refined (LOR) preconditioner, by setting their respective `low_order_refined` parameter to `true`. LOR solvers work by taking a problem and casting it onto a spectrally equivalent one with lower polynomial order and more refined mesh. Due to the scaling properties of the computing time with respect to polynomial order and mesh size, this change will often result in a significant performance improvement, which tends to be more pronounced at higher polynomial orders. More details can be found [here][https://mfem.org/pdf/workshop21/15_WillPazner_High_Order_Solvers.pdf]


!if-end!

!else
!include mfem/mfem_warning.md

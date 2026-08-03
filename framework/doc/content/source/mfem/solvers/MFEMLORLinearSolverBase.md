# MFEMLORLinearSolverBase

!if! function=hasCapability('mfem')

## Summary

Base class for `mfem::Solver` objects used as linear solvers or preconditioners in MFEM problems that
support variants for solving low-order-refined (LOR) problems.

## Overview

Classes derived from `LORLinearSolverBase<T>` represent preconditioners and/or linear solvers that
have LOR solver variants to accelerate the solution of high-order problems. Use of
these variants is controlled by setting the `low_order_refined` parameter to `true`. LOR solvers
work by taking a problem and casting it onto a spectrally equivalent one with lower polynomial order
and more refined mesh. Due to the scaling properties of the computing time with respect to
polynomial order and mesh size, this change will often result in a significant performance
improvement, which tends to be more pronounced at higher polynomial orders. More details can be
found [here](https://mfem.org/pdf/workshop21/15_WillPazner_High_Order_Solvers.pdf).

Any LOR-capable MFEM solver `T` must inherit from `LORLinearSolverBase<T>`. Provided methods are
targeted towards LOR solver variants described by `mfem::LORSolver<T>` objects; if this is not
appropriate for the desired LOR-based MFEM solver (for instance, if additional setup is required or
if building an `mfem::LORSolver<T>` object is unnecessary), the developer is free to specialize
`LORLinearSolverBase<T>::UpdateEquationSystemContext()` as required. With the `low_order_refined`
variant set to false (the default), the wrapped MFEM solver will be of type `T` and LOR functionality
will be disabled.

!alert note Solving a problem with vector variables with the LOR method requires a specific choice
of basis for the low-order and the high-order systems to be spectrally equivalent. Therefore, if
you are solving (a) an H1 problem or (b) an H(Curl) or H(Div) problem with an LOR solver, you must
set (a) `basis = GaussLobatto` (default) or (b) `closed_basis = GaussLobatto` (default) and
`open_basis = IntegratedGLL` in the corresponding `FESpaces` block.

!if-end!

!else
!include mfem/mfem_warning.md

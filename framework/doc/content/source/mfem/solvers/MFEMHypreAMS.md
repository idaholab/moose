# MFEMHypreAMS

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::HypreAMS` solver to use as a preconditioner or solver to solve the
MFEM equation system. Most effective for preconditioning and solving a curl-curl problem when using
Nédélec elements, in which case the $H(\mathrm{curl})$ FE space should be passed to the
`mfem::HypreAMS` solver during construction.

If the system of equations is singular - commonly arising, for example, when solving for the
magnetic vector potential in magnetostatic systems in the steady state - users should set the
`singular` parameter to `true` to add a small mass term to ensure solvability.

If only part of the domain has a zero mass term (e.g. a zero-conductivity region embedded in a
conducting domain), the `block` parameter may be set to the list of subdomains in which the mass
term vanishes. The solver then restricts the `HypreAMS` interior-node correction to the nodes
lying strictly inside those subdomains, excluding nodes on the interface with the rest of the
domain and on the domain boundary (which are constrained by the essential boundary condition).
The `projection_frequency` parameter controls how many iterations elapse between successive
projections of the iterate onto the compatible $H(\mathrm{curl})$ subspace; leaving it at `0`
disables periodic projection.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter
[!param](/Solvers/MFEMHypreAMS/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial
order systems.

## Example Input File Syntax

!listing test/tests/mfem/kernels/curlcurl.i block=FESpace Solvers

!syntax parameters /Solvers/MFEMHypreAMS

!syntax inputs /Solvers/MFEMHypreAMS

!syntax children /Solvers/MFEMHypreAMS

!if-end!

!else
!include mfem/mfem_warning.md

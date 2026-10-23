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
term vanishes. `HypreAMS` then regularizes the gradients of the nodal basis functions supported
in those subdomains, which span the null space of the operator there. Nodes on the interface
with the rest of the domain and on essential boundaries are excluded, because the gradients of
their basis functions are not in that null space. Nodes on boundaries with natural boundary
conditions are kept. An integrated boundary condition that adds a bilinear term on a boundary
of these subdomains, such as [MFEMVectorFEMassIntegratedBC.md], also removes those gradients
from the null space; it is not detected, so a subdomain adjoining such a boundary should not be
listed in `block`.

When `block` is set, `HypreAMS` also periodically projects the right-hand side of each
preconditioner application onto the compatible subspace orthogonal to these gradients. The
[!param](/Solvers/MFEMHypreAMS/projection_frequency) parameter sets the number of iterations
between projections; if it is unset, hypre projects every fifth iteration. Restricting to
subdomains is not supported together with
[!param](/Solvers/MFEMHypreAMS/low_order_refined), and cannot be combined with
[!param](/Solvers/MFEMHypreAMS/singular), which applies only when the mass term is zero
everywhere.

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

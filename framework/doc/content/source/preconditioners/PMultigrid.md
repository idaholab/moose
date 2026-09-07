# PMG

!if! function=hasCapability('kokkos')

!syntax description /Preconditioning/PMG

## Overview

`PMG` preconditions the Kokkos matrix-free partial-assembly Jacobian
(see [`use_kokkos_matrix_free_jacobian`](syntax/Executioner/index.md)) with a p-multigrid
hierarchy. Every level of the hierarchy is the fine system's variables at a reduced polynomial
order, on the fine mesh, evaluated at the fine quadrature rule. Because the quadrature rule is
shared, every level's operator is the same quadrature-point linearization the fine level computes
once per Newton step, contracted against that level's own basis functions, so the coarse levels
hold no kernels or boundary conditions of their own.

The orders of the coarse levels are listed ascending in `level_orders`; the fine level is the solver
system's own order and is not listed.

## Example Input File Syntax

!listing test/tests/kokkos/preconditioners/pmultigrid/pmultigrid.i block=Preconditioning

!syntax parameters /Preconditioning/PMG

!syntax inputs /Preconditioning/PMG

!syntax children /Preconditioning/PMG

!if-end!

!else
!include kokkos/kokkos_warning.md

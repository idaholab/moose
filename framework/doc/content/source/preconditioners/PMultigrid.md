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

Each level supplies the two operations a Jacobi or Chebyshev smoother needs: the action of its
operator on a vector, and the diagonal of that operator. Both are contractions of the same
quadrature-point linearization against the level's basis functions, so they agree with one another
to roundoff. Setting `verify_level_operators` checks that after every linearization by applying each
level's operator to one unit vector per degree of freedom, at the cost of an operator application per
degree of freedom, which makes it a verification aid for small inputs.

Consecutive levels are joined by a transfer that prolongs a coarse vector to the finer space and
restricts a fine vector back by the transpose of that same prolongation, which makes the operator the
hierarchy realizes on a coarse level the Galerkin operator of the fine linearization. For a family
whose nested spaces share no degrees of freedom, such as Lagrange, the transfer expands each coarse
element basis function in the finer element basis, so a transfer depends on the geometry and the
bases alone and is built once at initial setup. Setting `verify_level_transfers` checks the transpose
relationship of each transfer there, at the cost of one application of each direction.

## Symmetry and the outer Krylov method

For a steady system the operator is symmetric whenever the linearization it contracts is, which makes
CG a valid outer Krylov accelerator over the hierarchy. Symmetry follows from eliminating each degree
of freedom a Dirichlet boundary condition pins: its own row carries the identity, and its prescribed
value reaches every other row as data rather than as an unknown, so the matching column carries
nothing for the identity row to have to match. Setting `verify_operator_symmetry` measures the solver
system's own matrix-free operator against its transpose after every linearization, at the cost of one
operator application per degree of freedom, which makes it a verification aid for small inputs. A
solve that asks for CG should ask for this check alongside it.

A transient system carries the constrained columns, and with them an asymmetric operator. Its time
integrator forms the solution time derivative from the solution as the solver hands it over, so the
mass contribution to every row reads the pinned degree of freedom's own iterate value, and the
operator carries the matching column to remain the linearization of that residual. Requesting
`verify_operator_symmetry` for a transient system with Dirichlet-constrained degrees of freedom
reports an error; such a system is preconditioned with GMRES as the outer Krylov method.

## Example Input File Syntax

!listing test/tests/kokkos/preconditioners/pmultigrid/pmultigrid.i block=Preconditioning

!syntax parameters /Preconditioning/PMG

!syntax inputs /Preconditioning/PMG

!syntax children /Preconditioning/PMG

!if-end!

!else
!include kokkos/kokkos_warning.md

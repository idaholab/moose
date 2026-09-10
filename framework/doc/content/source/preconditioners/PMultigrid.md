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

## The coarsest level

The coarsest level is the only one that assembles a sparse matrix, and it is factored directly, by LU
through MUMPS so that the same configuration serves in parallel. A multigrid preconditioner has to be a
fixed linear operator: the outer Krylov method builds its space from repeated applications of it, and
relates the residual its recurrence tracks to the true residual on the assumption that the operator does
not change between applications. A direct factorization is such an operator exactly, and it is
affordable because the coarsest level of a p-hierarchy is a low-order space on the fine mesh.

Iterating that level instead fails whichever way the iteration is stopped. Stopping on a relative
tolerance makes the work, and so the operator, depend on the right-hand side. Stopping after a fixed
number of iterations fixes the work but not the operator, because a Krylov method builds its polynomial
from the Krylov space of the vector it is given: it is a nonlinear function of that vector however many
steps it runs. The symptom, when it happens, is an outer solve that reports convergence it has not
achieved, and a linear problem that takes several Newton steps.

## The smoother on each level

`smoother` selects what is applied on every level the hierarchy smooths rather than solves. The
default, `entity_block`, inverts the block of degrees of freedom each mesh entity carries: an
element's interior modes together, then each shared face, edge and vertex. The decomposition is read
off the degree-of-freedom map rather than constructed, so it follows whatever family and order the
level holds.

`point_jacobi` reads the operator diagonal alone. It is the cheaper application and the much weaker
smoother, because a modal basis puts several modes on one entity and a diagonal cannot represent the
coupling among them. The gap between the two widens with polynomial order, since the number of modes an
entity carries grows with it. On a hierarchic diffusion problem over a 16-by-16 mesh of biquadratic
elements, solved to a linear tolerance of 1e-8, with the coarse levels each row lists:

| fine order | coarse levels | `point_jacobi` | `entity_block` |
| - | - | - | - |
| 2 | 1 | 12 iterations | 12 iterations |
| 3 | 1 | 11 iterations | 11 iterations |
| 4 | 1, 2 | 79 iterations | 15 iterations |
| 8 | 1, 2, 4 | 819 iterations | 21 iterations |

The block smoother costs between a tenth and a quarter more per application, so those counts carry over
to time to solution: the two are within a fifth of one another at orders two and three, where a
hierarchic entity carries a single mode and the two smoothers coincide, while the block smoother is
about four times faster at order four and thirty-seven times faster at order eight. That is why it is
the default, and `point_jacobi` is worth selecting only for a low-order hierarchy where the counts
agree.

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

# ActuallyExplicitEuler

!syntax description /Executioner/TimeIntegrator/ActuallyExplicitEuler

## Description

`ActuallyExplicitEuler` implements true, first-order, forward Euler.  It differentiates itself from [/ExplicitEuler.md] by implementing solution methods that don't involve the "nonlinear" solver in MOOSE at all and, in the case of `lumped`, actually don't involve a linear solve at all.  This makes it incredibly fast.

## `solve_type`

This object has three different ways of solving built-in.  You can switch between them by using the `solve_type` parameter.  The three methods are:

1.  `consistent`: (The Default) A full mass matrix is built and used in a linear solve for the update
2.  `lumped`: A "lumped" mass matrix is built, hand-inverted and applied to the RHS - fast, but possibly less accurate
3.  `lump_preconditioned`: The inversion of the "lumped" mass matrix is used to precondition the `consistent` solve.  Right now this is still experimental.

All three methods are solved similarly: by solving for the update $\delta u$ then adding that to the existing solution.

Below is some more explanation of each of these `solve_type` options:

### `consistent`

The `consistent` option builds a full ("consistent") "mass matrix" and uses it in a linear solve to get the update.  This is done by calling `FEProblem::computeJacobianTag()` and specifying the `TIME` tag which includes all of the `TimeKernel` derived Kernels and `NodalBC` derived BoundaryConditions to compute $\mathbf{M}$.  A residual computation is also completed for the `NONTIME` tag to use as the RHS creating the equation:

\begin{equation}
\mathbf{M} \delta u = -R
\end{equation}

This is solved using the default linear solver from libMesh (usually PETSc... including the application of any command-line parameters specified).

### `lumped`

The `lumped` option creates a "lumped mass matrix" to use in the solve.  A lumped mass matrix is a digaonal matrix where the diagonal is the sum of all elements on the row from the original matrix.

To achieve the lumping here a matrix-vector product is performed with a vector of all ones.  That creates a vector where each entry is what would be on the diagonal of the lumped mass matrix.

The inverse of a diagonal matrix is simply the reciprocal of each diagonal entry - easily applied to our vector.  Then the matrix-vector product of the "inverse" lumped diagonal matrix is applied by simply doing a pointwise multiplication with the RHS.

This means that the `lumped` option actually doesn't need to solve a system of linear equations at all... making it incredibly fast.  However, the use of a lumped mass matrix may lead to unacceptable phase errors.

### `lump_preconditioned`

This option is the combination of the above two.  The consistent mass matrix is built and used to solve... but the preconditioner is applied as simply the inverse of the lumped mass matrix.  This means that solving the true (consistent) system can be done with simply using point-wise multiplications.  This makes it incredibly fast and memory efficient while still accurate.

!syntax parameters /Executioner/TimeIntegrator/ActuallyExplicitEuler

!syntax inputs /Executioner/TimeIntegrator/ActuallyExplicitEuler

!syntax children /Executioner/TimeIntegrator/ActuallyExplicitEuler

!bibtex bibliography

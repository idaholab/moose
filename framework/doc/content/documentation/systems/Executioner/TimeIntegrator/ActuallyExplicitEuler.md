# ActuallyExplicitEuler

!syntax description /Executioner/TimeIntegrator/ActuallyExplicitEuler

## Description

`ActuallyExplicitEuler` implements true, first-order, forward Euler.  It differentiates itself from [/ExplicitEuler.md] by implementing solution methods that don't involve the "nonlinear" solver in MOOSE at all and, in the case of `lumped`, actually don't involve a linear solve at all.  This makes it incredibly fast.

## `solve_type`

This object has three different ways of solving.  You can switch between them by using the `solve_type` parameter.  The three methods are:

1.  `consistent`: (The Default) A full mass matrix is built and used in a linear solve for the update
2.  `lumped`: A "lumped" mass matrix is built, hand-inverted and applied to the RHS - fast, but possibly less accurate
3.  `lump_preconditioned`: The inversion of the "lumped" mass matrix is used to precondition the `consistent` solve.  Right now this is still experimental.

All three methods are solved similarly: by solving for the update $\delta u$ then adding that to the existing solution.

Below is some more explanation of each of these `solve_type` options:

### `consistent`

The `consistent` option builds a full ("consistent") "mass matrix" and uses it in a linear solve to get the update.  This is done by calling `FEProblem::computeJacobianTag()` and specifying the `TIME` tag which includes all of the `TimeKernel` derived Kernels and `NodalBC` derived BoundaryConditions to compute $\mathbf{M}$:

!listing framework/src/timeintegrators/ActuallyExplicitEuler.C line=computeJacobianTag

A residual computation is also completed to use as the RHS ($R$):

!listing framework/src/timeintegrators/ActuallyExplicitEuler.C line=computeResidual

Creating the equation:

\begin{equation}
\mathbf{M} \delta u = -R
\end{equation}

This is solved using the default linear solver from libMesh (usually PETSc... including the application of any command-line parameters specified).

### `lumped`

The `lumped` option creates a "lumped mass matrix" to use in the solve.  A lumped mass matrix is a diagonal matrix where the diagonal is the sum of all elements on the row from the original matrix.

To achieve the lumping here a matrix-vector product is performed with a vector of all ones.  That creates a vector where each entry is what would be on the diagonal of the lumped mass matrix.

The inverse of a diagonal matrix is simply the reciprocal of each diagonal entry - easily applied to our vector.  Then the matrix-vector product of the "inverse" lumped diagonal matrix is applied by simply doing a pointwise multiplication with the RHS.

This means that the `lumped` option actually doesn't need to solve a system of linear equations at all... making it incredibly fast.  However, the use of a lumped mass matrix may lead to unacceptable phase errors.

### `lump_preconditioned`

This option is the combination of the above two.  The consistent mass matrix is built and used to solve... but the preconditioner is applied as simply the inverse of the lumped mass matrix.  This means that solving the true (consistent) system can be done with simply using point-wise multiplications.  This makes it incredibly fast and memory efficient while still accurate.

## Advanced Details

A few notes on some of the implementation details of this object:

### Update Form

Note that even though we're doing an explicit solve we are currently doing it in "update form" similar to a single step Newton solve.  This gives us good parity with the rest of MOOSE.  We may want to change this in the future to make better use of the fact that the mass-matrix can be constant for a wider class of problems if we remove `dt` from it.

### `_ones`

To get the sum of each row of the mass matrix for "lumping" purposes a vector consisting of all `1`s is used in a matrix-vector product:

!listing framework/src/timeintegrators/ActuallyExplicitEuler.C line=mass_matrix.vector_mult

This is actually the very same way `MatGetRowSum` is implemented in PETSc.  Doing it ourselves though cuts down on vector creation/destruction and a few other bookkeeping bits.

In the future we might change this to use `MatGetRowSum` if a specialization for `MPI_Aij` format is created

### Time

Time in an explicit solve must be handled carefully.  When evaluating the weak form (the integral parts) of the residual time needs to actually be the "old" time (the time we are solving "from"):

!listing framework/src/timeintegrators/ActuallyExplicitEuler.C line=timeOld

However, `DirichletBC` derived boundary conditions need to use the **final** time to evaluate themselves.  Think of it this way: you're integrating forward the "forces" as if evaluated from the beginning of the step... but ultimately the value on the boundary must end up being what it is supposed to be at the final time... no matter what.  To achieve that we reset time to the `_current_time` in-between weak form evalution and `NodalBC` boundary condition application in `postResidual()`.  `postResidual()` gets called at exactly this time to allow us to combine the `time` and `nontime` residuals into a single residual.  So it's convenient to simply do:

!listing framework/src/timeintegrators/ActuallyExplicitEuler.C line=_fe_problem.time() = _current_time;

After `postResidual()` the `NodalBC` BCs will get applied with the time at the final time for the step.

### `meshChanged()`

When the mesh changes the linear solver needs to be destroyed and recreated.  This is done by simply building a new one and setting it up again.  This happens automatically just by "overwriting" the `std::unique_ptr` to the LinearSolver.

### `lump_preconditioned`

The `lump_preconditioned` option invokes a `LumpedPreconditioner` helper object:

!listing framework/src/timeintegrators/ActuallyExplicitEuler.C line=class LumpedPreconditioner

This helper object simply applies the inverse of the diagonal, lumped mass-matrix as the preconditioner for the linear solve.  This is extremely efficient.  Note that when this option is applied you shouldn't specify any other preconditioners using command-line syntax or they will override this option.  In my testing this worked well.

!syntax parameters /Executioner/TimeIntegrator/ActuallyExplicitEuler

!syntax inputs /Executioner/TimeIntegrator/ActuallyExplicitEuler

!syntax children /Executioner/TimeIntegrator/ActuallyExplicitEuler

!bibtex bibliography

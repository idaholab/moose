# ExplicitTimeIntegrator

## Description

`ExplicitTimeIntegrator` is a base class for explicit time integrators that are
implemented without performing any nonlinear solve, which reduces runtime. Unlike
explicit time integrators that are not derived from this class, it is not
necessary to set `implicit` to `false` for all of the non-time residual objects.

## Methods of Solution

Time integrators deriving from this class have three solve options, provided via
the `solve_type` parameter:

- `consistent`: (the default) A full mass matrix is built and used in a linear solve for the update
- `lumped`: A "lumped" mass matrix is built, inverted, and applied to the RHS, which
  is faster but can possibly be less accurate.
- `lump_preconditioned`: The inversion of the "lumped" mass matrix is used to
  precondition the `consistent` solve.

All three methods are solved similarly: a linear solve is performed to obtain a
solution update $\delta u$ that is added to the existing solution.

Below is some more explanation of each of these `solve_type` options:

### `consistent`

The `consistent` option builds a full ("consistent") "mass matrix" and uses it
in a linear solve to get the update.  This is done by calling
`FEProblem::computeJacobianTag()` and specifying the `TIME` tag which includes
all of the `TimeKernel` derived Kernels and `NodalBC` derived BoundaryConditions
to compute $\mathbf{M}$:

!listing framework/src/timeintegrators/ActuallyExplicitEuler.C line=computeJacobianTag

A residual computation is also completed to use as the RHS ($R$):

!listing framework/src/timeintegrators/ActuallyExplicitEuler.C line=computeResidual

Finally, the following linear system is solved to obtain the solution update
$\delta u$ using the default linear solver from libMesh (usually PETSc, including
the application of any command-line parameters specified):

\begin{equation}
\mathbf{M} \delta u = -R
\end{equation}

### `lumped`

The `lumped` option creates a "lumped" mass matrix to use in the solve.  A lumped mass matrix is a diagonal matrix where the diagonal is the sum of all elements on the row from the original matrix.
Here, to achieve the lumping, a matrix-vector product is performed between the
consistent mass matrix and a vector of all ones.

The inverse of a diagonal matrix is simply the reciprocal of each diagonal entry.
Then the matrix-vector product of the "inverse" lumped diagonal matrix is applied by simply doing a pointwise multiplication with the RHS.

Thus the `lumped` option does not actually solve a system of linear equations,
allowing it be much faster. However, the lumping of the mass matrix may lead to
unacceptable phase errors.

### `lump_preconditioned`

This option is the combination of the other two options: the consistent mass matrix
is used in the linear system, but the linear solve is preconditioned using the
lumped mass matrix. This compromise retains the accuracy of the `consistent`
option while benefiting from some of the speedup offered by the `lumped` option.

The lumped mass matrix preconditioner is applied with the class, `LumpedPreconditioner`:

!listing framework/include/timeintegrators/LumpedPreconditioner.h

This object simply applies the inverse of the diagonal, lumped
mass-matrix as the preconditioner for the linear solve, which is very
efficient. Note that when this option is applied you shouldn't specify any other
preconditioners using command-line syntax or they will override this option.

## Additional Details

Some notes on some of the implementation details of this class follow.

### Update Form

Note that even though we're doing an explicit solve we are currently doing it in
"update form" similar to a single step Newton solve.  This gives us good parity
with the rest of MOOSE.  We may want to change this in the future to make better
use of the fact that the mass-matrix can be constant for a wider class of
problems if we remove `dt` from it.

### `_ones`

To get the sum of each row of the mass matrix for "lumping" purposes a vector consisting of all `1`s is used in a matrix-vector product:

!listing framework/src/timeintegrators/ExplicitTimeIntegrator.C line=mass_matrix.vector_mult

This is actually the very same way `MatGetRowSum` is implemented in PETSc; however,
doing it manually cuts down on vector creation/destruction and a few other book-keeping operations.

In the future this could be changed to use `MatGetRowSum` if a specialization for `MPI_Aij` format is created.

### Time

Time in an explicit solve must be handled carefully. When evaluating the weak
form (the integral parts) of the residual, time needs to be set to be the "old"
time (the time we are solving "from"):

!listing framework/src/timeintegrators/ActuallyExplicitEuler.C line=timeOld

However, `DirichletBC` derived boundary conditions need to use the **final** time,
since the strong constraints they represent use the final time and are not affected
by the time integrator. To achieve this, time is reset to the `_current_time` after
the weak form residual evaluation and before `NodalBC` boundary condition application,
which makes `postResidual()` the correct place to reset time for this purpose:

!listing framework/src/timeintegrators/ActuallyExplicitEuler.C line=_fe_problem.time() = _current_time;

After `postResidual()` the `NodalBC` BCs are applied with the time at the final time for the step.

### `meshChanged()`

When the mesh changes the linear solver needs to be destroyed and recreated.
This is done by simply building a new one and setting it up again.  This happens
automatically just by "overwriting" the `std::unique_ptr` to the LinearSolver.

### Relevant Executioner solver options

You can ignore this section if using `solve_type = lumped`. No [Executioner.md]
parameters are relevant to you in that case. However, for `consistent` or
`lump_preconditioned` solve types, the `l_tol` and `l_max_its` parameters are
used in the solution process. Nonlinear executioner options are not
relevant. When using PETSc as the default solver package, `pc` and `ksp` options
from the `petsc_options*` parameters will be used while `snes` options will be
ignored.

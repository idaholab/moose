# ReferenceResidualProblem

!syntax description /Problem/ReferenceResidualProblem

## Description

By default, MOOSE checks convergence using relative and absolute criteria. Once the residual drops
below either an absolute tolerance, or the residual divided by the initial residual for the current
time step drops below a relative tolerance, the solution is considered converged. This works well for
many problems, but there are some scenarios that are problematic for convergence:

1. There is a large scaling difference between the solution variables, so taking the $L^2$ norm of the
   combined vector skews the convergence check so that it becomes increasingly dominated by the variable
   with larger absolute values in the residual vector as the scaling difference increases.

1. The model conditions have changed very little from the previous step, so the initial residual is very
   low because the solution is essentially converged before any nonlinear iterations have been performed.
   The standard convergence check that uses the initial residual as a reference quantity eventually fails
   after a few steps because the solution is forced to converge to tighter limits with each time
   step, until numerical roundoff error prevents the solution from converging more tightly.

1. The state of the problem is such that the values of the residual at all degrees of freedom where
   Dirichlet boundary conditions are applied are zero in the converged solution. An example of this is a
   mechanics model that has Dirichlet boundary conditions to prevent rigid body motion, but the model
   only experiences free thermal expansion, so there are no reaction loads at those points. Another
   example is if there is a time step when no loading is applied.


`ReferenceResidualProblem` checks for convergence by comparing the residual to a different
reference quantity (instead of the initial residual). The user specifies a reference vector that can be used in
a relative convergence check. Because the solution variables can have significantly different
scaling, the convergence check performed in `ReferenceResidualProblem` checks convergence of the
solution variables individually. When the $L^2$ norm of the residual for each solution variable is
less than either the relative tolerance times the $L^2$ norm of the corresponding reference variable
or the absolute tolerance, the solution is considered converged.

By checking the convergence of individual variables and comparing to reference quantities that are
meaningful even when the solution is not changing, `ReferenceResidualProblem` addresses the first two
issues listed above. The third issue is potentially more of a problem with `ReferenceResidualProblem`
than the standard MOOSE convergence check, and dealing with
it is discussed in [#zero_resid].

Use of this procedure requires that the user provide physically meaningful reference quantities. The
vector of the reaction loads (in the case of mechanics) or integrated fluxes (in the case of
diffusion problems) is typically suitable for this purpose, as it provides a measure of the loading
applied to the system. To make these reference quantities, simply add the
`extra_vector_tags = <reference_vector_name>` param/value pair to the computing objects that you
want to add into the reference vector. An explicit example is given in [#example] below.

Since relative convergence is computed differently with this approach, the nonlinear relative
tolerance required to achieve the same error is typically different than with the default approach in
MOOSE, and the differences will vary by the problem. The code user must evaluate the behavior of
their model to ensure that appropriate tolerances are being used.

### Dealing with Residuals that are Zero at Boundaries id=zero_resid

As mentioned in the third case above where relative convergence checks can fail,
the converged solutions for some problems are zero on all boundaries with Dirichlet
boundary condition (BC) because of the nature of the loading conditions. This would
make the `ReferenceResidualProblem` convergence checks fail because the norm of the
reference vector would be zero in this case.  This is manifested when the reference
values decrease at the same rate as the residuals, so relative convergence is never
achieved.

In some mechanics problems, such as those with symmetry planes, this might happen
only in one direction. This situation can be remedied by grouping together the
residuals for all components of the displacement variables rather than checking
them individually. If the reference vector for one of the variables in the group
is zero (because there are no Dirichlet BCs that restrict the solution in that direction),
but it is nonzero for the others, the residual for the set of variables will
be checked together, and the variable with a zero value will not prevent
convergence as it normally would.  Grouping variables is accomplished by
using the `group_variables` option, which is used to provide one or more
lists of names of variables to group together.

In some cases, however, there may be no degrees of freedom with nonzero residuals
at Dirichlet BCs for even a group of variables.  This could happen in mechanics
problems that only have free expansion, in which the Dirichlet BCs are important
for preventing rigid body motion, but do not restrain the deformation of the body.
This could also happen if there is simply no loading on a model during a time step.

It is necessary in such cases to also specify an absolute tolerance using `nl_abs_tol`,
which is a problem-specific value that must be higher than the residual below which
the solution can no longer converge tighter due to numeric roundoff errors, but sufficiently
low to ensure that the solution is converged when that limit is reached.

## Example Input Syntax id=example

!listing test/tests/problems/reference_residual_problem/reference_residual.i block=Problem

where the `extra_tag_vectors` parameter indicates the additional vectors that should be added to the
nonlinear system. This parameter must contain the name of the vector to be used for the
`reference_vector`. In this example we only create one extra vector, the `ref` vector, that will be
used for holding the reference residuals. To have computing objects add into the reference vector,
simply add the `extra_vector_tags = <reference_vector_name>` param/value pair as illustrated below:

!listing test/tests/problems/reference_residual_problem/reference_residual.i block=BCs

In this example we are using the integrated fluxes as the reference quantities that we will compare
the individual variable residuals to.

### Grouping Variables

!listing modules/combined/test/tests/reference_residual/group_variables.i block=Problem

Multiple groupings of variables can be provided in `group_variables` by separating them by  semicolon.
Convergence for those variables that are not given in `group_variables` is checked individually. A given variable can only be included in one group.

!syntax parameters /Problem/ReferenceResidualProblem

!syntax inputs /Problem/ReferenceResidualProblem

!syntax children /Problem/ReferenceResidualProblem

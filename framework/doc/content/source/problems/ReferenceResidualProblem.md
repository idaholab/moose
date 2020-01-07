# Reference Residual Problem

!syntax description /Problem/ReferenceResidualProblem

## Description

By default, MOOSE checks convergence using relative and absolute criteria. Once the residual drops
below either an absolute tolerance, or the residual divided by the initial residual for the current
time step drops below a relative tolerance, the solution is considered converged. This works well for
many problems, but in cases where the solution changes minimally between time steps, the initial
residual can be very small, making the relative convergence check much too stringent.

The `ReferenceResidualProblem` checks for convergence by comparing the residual to a different
reference quantity (instead of the initial residual). The user specifies a reference vector that can be used in
a relative convergence check. Because the solution variables can have significantly different
scaling, the convergence check performed in `ReferenceResidualProblem` checks convergence of the
solution variables individually. When the $l^2$ norm of the residual for each solution variable is
less than either the relative tolerance times the $l^2$ norm of the corresponding reference variable
or the absolute tolerance, the solution is considered converged.

For some cases, it is more appropriate to group several variables together to check convergence of their grouped solution rather than checking the convergence of the variables individually.
For example, in 2D and 3D mechanics solutions, there are multiple displacement variables, and because they have similar physical meaning, it often makes sense to treat them as one vector,
which helps avoid issues that may be encountered if the model configuration is such that the reactions in one direction are much lower than in other directions.

When variables are grouped together, the $l^2$ norm of the grouped residual for those variables is used to check convergence.
The optional parameter `group_variables` is used to provide one or more lists of variable names which are to be grouped together.

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

## Example Input syntax id=example

!listing test/tests/problems/reference_residual_problem/reference_residual.i block=Problem

where the `extra_tag_vectors` parameter indicates the additional vectors that should be added to the
non-linear system. This parameter must contain the name of the vector to be used for the
`reference_vector`. In this example we only create one extra vector, the `ref` vector, that will be
used for holding the reference residuals. To have computing objects add into the reference vector,
simply add the `extra_vector_tags = <reference_vector_name>` param/value pair as illustrated below:

!listing test/tests/problems/reference_residual_problem/reference_residual.i block=BCs

In this example we are using the integrated fluxes as the reference quantites that we will compare
the invidual variable residuals to.

## Example Input syntax to group variables

!listing modules/combined/test/tests/reference_residual/group_variables.i block=Problem

Multiple groupings of variables can be provided in `group_variables` by separating them by  semicolon.
Convergence for those variables that are not given in `group_variables` is checked individually. A given variable can only be included in one group.

!syntax parameters /Problem/ReferenceResidualProblem

!syntax inputs /Problem/ReferenceResidualProblem

!syntax children /Problem/ReferenceResidualProblem

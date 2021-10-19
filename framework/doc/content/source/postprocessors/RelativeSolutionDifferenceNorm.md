# RelativeSolutionDifferenceNorm

!syntax description /Postprocessors/RelativeSolutionDifferenceNorm

The formula for the relative difference is:

!equation
\dfrac{u^{n+1} - u^n}{u^n}

where $u$ is the solution vector, composed of the degrees of freedom of all the components of all the nonlinear variables,
and $n$ is the index of the time step.

## Example input syntax

In this example, the `RelativeSolutionDifferenceNorm` is used to examine the
convergence of a transient source-diffusion problem.

!listing test/tests/postprocessors/relative_solution_difference_norm/test.i block=Postprocessors

!syntax parameters /Postprocessors/RelativeSolutionDifferenceNorm

!syntax inputs /Postprocessors/RelativeSolutionDifferenceNorm

!syntax children /Postprocessors/RelativeSolutionDifferenceNorm

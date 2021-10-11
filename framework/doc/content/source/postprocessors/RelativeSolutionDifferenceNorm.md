# RelativeSolutionDifferenceNorm

!syntax description /Postprocessors/RelativeSolutionDifferenceNorm

The formula for the relative difference is:

!equation
\dfrac{current\_solution - old\_solution}{current\_solution}

## Example input syntax

In this example, the `RelativeSolutionDifferenceNorm` is used to examine the
convergence of a transient source-diffusion problem.

!listing test/tests/postprocessors/relative_solution_difference_norm/test.i block=Postprocessors

!syntax parameters /Postprocessors/RelativeSolutionDifferenceNorm

!syntax inputs /Postprocessors/RelativeSolutionDifferenceNorm

!syntax children /Postprocessors/RelativeSolutionDifferenceNorm

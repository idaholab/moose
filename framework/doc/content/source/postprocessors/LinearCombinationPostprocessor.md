# LinearCombinationPostprocessor

## Description

This post-processor computes a linear combination between an arbitrary number of post-processors $x_i$:
\begin{equation}
  y = \sum\limits_i c_i x_i + b \,,
\end{equation}
where $c_i$ is the combination coefficient for $x_i$, and $b$ is an additional
value to add to the sum.

## Example Syntax

The following example demonstrates how this post-processor is used:

!listing test/tests/postprocessors/linear_combination/linear_combination.i
block=Postprocessors

!syntax parameters /Postprocessors/LinearCombinationPostprocessor

!syntax inputs /Postprocessors/LinearCombinationPostprocessor

!syntax children /Postprocessors/LinearCombinationPostprocessor

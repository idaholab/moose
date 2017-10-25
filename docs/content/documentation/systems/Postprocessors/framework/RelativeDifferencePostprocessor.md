# RelativeDifferencePostprocessor

## Description

This post-processor computes the absolute value of the relative difference
between 2 post-processor values:
\begin{equation}
  y = \left| \frac{x_1 - x_2}{x_1} \right| \,,
\end{equation}
where $x_1$ and $x_2$ are the 2 post-processor values. Note that $x_2$ is used
as the base for the relative difference. If $x_2 \approx 0$, then the absolute
difference is used instead to prevent division by zero:
\begin{equation}
  y = \left| x_1 - x_2 \right| \,.
\end{equation}

## Example Syntax

The following example demonstrates how this post-processor is used:

!listing test/tests/postprocessors/relative_difference/relative_difference.i
block=Postprocessors

!syntax parameters /Postprocessors/RelativeDifferencePostprocessor

!syntax inputs /Postprocessors/RelativeDifferencePostprocessor

!syntax children /Postprocessors/RelativeDifferencePostprocessor

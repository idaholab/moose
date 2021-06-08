# ChangeOverFixedPointPostprocessor

## Description

This post-processor computes the change or relative change in a post-processor
value over a fixed point iterations. It may be used to track the convergence of
the fixed point algorithm.
\begin{equation}
  (\Delta y)^n = y^n - y_{ref} \,,
\end{equation}
where $(\Delta y)^n$ is the relative change at iteration $n$, $y^n$ is the
value of post-processor $y$ at iteration $n$, and $y_{ref}$ is the reference
value for the change (either the previous iteration value, $y^{n-1}$, or the
initial value at the beginning of the iteration process, $y^0$. Relative change
is computed as
\begin{equation}
  (\Delta y)^n = \frac{y^n - y_{ref}}{y_{ref}} \,.
\end{equation}
Optionally, the user may specify to return the absolute value of the change.

## Example Syntax

The following example demonstrates how this post-processor is used:

!listing test/tests/postprocessors/change_over_time/change_over_time.i
block=Postprocessors

!syntax parameters /Postprocessors/ChangeOverFixedPointPostprocessor

!syntax inputs /Postprocessors/ChangeOverFixedPointPostprocessor

!syntax children /Postprocessors/ChangeOverFixedPointPostprocessor

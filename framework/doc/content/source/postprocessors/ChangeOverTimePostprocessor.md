# ChangeOverTimePostprocessor

## Description

This post-processor computes the change or relative change in a post-processor
value over a time step or over the entire transient. The change is computed as
\begin{equation}
  (\Delta y)^n = y^n - y_{ref} \,,
\end{equation}
where $(\Delta y)^n$ is the relative change at time index $n$, $y^n$ is the
value of post-processor $y$ at time index $n$, and $y_{ref}$ is the reference
value for the change (either the previous time value, $y^{n-1}$, or the initial
value, $y^0$). The relative change, which may be requested instead using the
[!param](/Postprocessors/ChangeOverTimePostprocessor/compute_relative_change)
parameter is computed as

\begin{equation}
  (\Delta y)^n = \frac{y^n - y_{ref}}{y_{ref}} \,.
\end{equation}
Optionally, the user may specify to return the absolute value of the change.

## Example Syntax

The following example demonstrates how this post-processor is used:

!listing test/tests/postprocessors/change_over_time/change_over_time.i
block=Postprocessors

!syntax parameters /Postprocessors/ChangeOverTimePostprocessor

!syntax inputs /Postprocessors/ChangeOverTimePostprocessor

!syntax children /Postprocessors/ChangeOverTimePostprocessor

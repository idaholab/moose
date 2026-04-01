# ChangeOverTimePostprocessor

## Description

This post-processor computes the change in a post-processor
value over a time step or over the entire transient. The change is computed as

!equation
(\Delta y)_n = y_n - y_\text{ref} \,,

where $(\Delta y)_n$ is the change of $y$ at time index $n$, $y_n$ is the
value of post-processor $y$ at time index $n$, and $y_\text{ref}$ is the reference
value for the change (either the previous time value, $y_{n-1}$, or the initial
value, $y_0$). The reference value is determined by [!param](/Postprocessors/ChangeOverTimePostprocessor/change_with_respect_to_initial):

- `true`: $y_\text{ref} = y_0$
- `false`: $y_\text{ref} = y_n$

The parameter [!param](/Postprocessors/ChangeOverTimePostprocessor/compute_relative_change)
is used to divide the result by the reference value to yield a relative change:

!equation
\left(\frac{\Delta y}{y}\right)_n = \frac{y_n - y_\text{ref}}{y_\text{ref}} \,.

The parameter [!param](/Postprocessors/ChangeOverTimePostprocessor/divide_by_dt) may
be used to estimate the time derivative of the post-processor value by dividing by
the time step size:

!equation
\left(\frac{\Delta y}{\Delta t}\right)_n = \frac{y_n - y_{n-1}}{\Delta t_n} \,.

Note that this requires [!param](/Postprocessors/ChangeOverTimePostprocessor/change_with_respect_to_initial) to be `false`.

The parameter [!param](/Postprocessors/ChangeOverTimePostprocessor/take_absolute_value)
may be used to take the absolute value of the result.

## Example Syntax

The following example demonstrates how this post-processor is used:

!listing test/tests/postprocessors/change_over_time/change_over_time.i
block=Postprocessors

!syntax parameters /Postprocessors/ChangeOverTimePostprocessor

!syntax inputs /Postprocessors/ChangeOverTimePostprocessor

!syntax children /Postprocessors/ChangeOverTimePostprocessor

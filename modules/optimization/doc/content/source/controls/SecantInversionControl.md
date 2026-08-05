# SecantInversionControl

!syntax description /Controls/SecantInversionControl

## Overview

`SecantInversionControl` helps solve a scalar inverse problem inside a fixed-point (Picard) iteration:
at each time step it adjusts a parameter `p` (held in a [Receiver](Receiver.md) postprocessor and
transferred to a sub-application) so that a sub-application output postprocessor matches a target
[Function](Functions/index.md) of time.

Each fixed-point iteration, after the sub-application has solved with the current `p` and its output
has been transferred back, this control performs a single secant (quasi-Newton) update:

!equation
p_{k+1} = p_k - (y_k - y_\text{target}) \frac{p_k - p_{k-1}}{y_k - y_{k-1}}

On the first iteration of each sweep only one `(p, y)` pair is available, so the update is seeded by
perturbing `p` by `initial_delta`. The outer iteration count, convergence test, and time-step
cutting are owned by the [Executioner](Executioner/index.md) and the
[Convergence](Convergence/index.md) system (typically a [PostprocessorConvergence](PostprocessorConvergence.md)
on the output residual), not by this control.

The optional `converged_parameter_postprocessor` receives the parameter value that produced the
current output; at convergence it holds the inverse-problem solution.

## Example Input Syntax

!listing test/tests/controls/inverse_solve/main.i block=Controls Convergence

!syntax parameters /Controls/SecantInversionControl

!syntax inputs /Controls/SecantInversionControl

!syntax children /Controls/SecantInversionControl

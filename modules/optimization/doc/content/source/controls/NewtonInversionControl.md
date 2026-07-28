# NewtonInversionControl

!syntax description /Controls/NewtonInversionControl

## Overview

`NewtonInversionControl` helps solve a scalar inverse problem inside a fixed-point (Picard) iteration:
at each time step it adjusts a parameter $p$ (held in a [Receiver](Receiver.md) postprocessor and
transferred to a sub-application) so that a sub-application output postprocessor matches a target
[Function](Functions/index.md) of time. It is the finite-difference Newton companion to
[SecantInversionControl](SecantInversionControl.md).

### Fixed-point Iterations Handling

A `Control` cannot safely drive its own perturbed sub-application solve: the fixed-point executioner
owns the sub-application's single backup slot and restores it between iterations, so an extra
control-driven `backup()`/`solveStep()` would corrupt that state. Instead, the local finite-
difference derivative is formed over **two consecutive fixed-point iterations**, both of which the
framework starts from the same start-of-step state:

- +Base iteration+ (parameter $p_{base}$): record the output $y_{base}$, publish $p_{base}$ as the
  solution of record, report the normalized residual, and set the parameter to
  $p_{base} + \delta p$ for the next solve.
- +Perturbed iteration+ (parameter $p_{base} + \delta p$): form
  $df/dp = (y - y_{base}) / (p - p_{base})$ and take one Newton step
  $p_{next} = p_{base} - (y_{base} - y_{target}) / (df/dp)$. A large `nonconverged_residual` sentinel is
  written so convergence is only ever declared on a base iteration, guaranteeing the recorded
  solution is an un-perturbed parameter that actually produced the converged output.

The outer iteration count, convergence test, and time-step cutting are owned by the
[Executioner](Executioner/index.md) and the [Convergence](Convergence/index.md) system (a
[PostprocessorConvergence](PostprocessorConvergence.md) on the control-written `residual_postprocessor`,
compared against a tolerance of 1). Because each Newton step spans two fixed-point iterations, `max_iterations` on the
Convergence object must allow roughly twice the number of Newton steps.

## Example Input Syntax

!listing test/tests/controls/newton_inverse_solve/main_newton.i block=Controls Convergence

!syntax parameters /Controls/NewtonInversionControl

!syntax inputs /Controls/NewtonInversionControl

!syntax children /Controls/NewtonInversionControl

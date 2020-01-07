# TimeIntegrator System

The MOOSE framework implements a number of time integration methods, which
include both explicit and implicit methods, of varying truncation error orders
and stability properties:

!syntax list /Executioner/TimeIntegrator objects=True actions=False subsystems=False

## Execution of a Time Step

For most time integrators, at least one nonlinear solve is required per time
step; for others (e.g., [ActuallyExplicitEuler](ActuallyExplicitEuler.md)), only
a linear solve is required. If solution fails, then that time step is aborted,
and depending on parameters of the [Executioner](Executioner/index.md) and
[TimeStepper](TimeStepper/index.md), the transient may abort completely or just
decrease the time step size and try again.

!alert note title=Multi-stage time integrator implementation details
For multi-stage time integrators such as [ExplicitTVDRK2](ExplicitTVDRK2.md),
a check is performed after each stage to verify nonlinear convergence; if the
nonlinear system is +not+ converged, then the time step is aborted immediately.
This is achieved by simply returning prematurely from the `solve()` function
and allowing the post-`solve()` convergence check to report failure, since the
convergence flag should still be set to `false`. If convergence checks are not
after each stage, then only the last stage will be required to converge; other
stages could just reach the maximum number of nonlinear iterations without
convergence and proceed to the next stage anyway.

## Actions

Actions associated with the `TimeIntegrator` system are the following:

!syntax list /Executioner/TimeIntegrator objects=False actions=True subsystems=False

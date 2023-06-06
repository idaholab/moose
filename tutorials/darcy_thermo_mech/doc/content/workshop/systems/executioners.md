# [Executioner System](syntax/Executioner/index.md)

A system for dictating the simulation solving strategy.

!---

## Steady-state Executioner

Steady-state executioners generally solve the nonlinear system just once.

!listing steady_time.i block=Executioner

The Steady executioner can solve the nonlinear system multiple times while adaptively
refining the mesh to improve the solution.

!---

## Transient Executioners

Transient executioners solve the nonlinear system at least once per time step.

| Option | Definition
| :- | :- |
| `dt` | Starting time step size |
| `num_steps` | Number of time steps |
| `start_time` | The start time of the simulation |
| `end_time` | The end time of the simulation |
| `scheme` | Time integration scheme (discussed next) |


!listing executioner/transient.i block=Executioner

!---

### Steady-State Detection

| Option | Definition |
| :- | :- |
| `steady_state_detection` | Whether to try and detect achievement of steady-state (Default = `false`) |
| `steady_state_tolerance` | Used for determining a steady-state; Compared against the difference in solution vectors between current and old time steps (Default = `1e-8`) |

!---

## Common Executioner Options

There are a number of options that appear in the executioner block and are used to control the
solver. Here are a few common options:

| Option | Definition |
| :- | :- |
| `l_tol` | Linear Tolerance (default: 1e-5) |
| `l_max_its` | Max Linear Iterations (default: 10000) |
| `nl_rel_tol` | Nonlinear Relative Tolerance (default: 1e-8) |
| `nl_abs_tol` | Nonlinear Absolute Tolerance (default: 1e-50) |
| `nl_max_its` | Max Nonlinear Iterations (default: 50) |


!---

## TimeKernel Object

The TimeKernel object adds two member variables to a Kernel object:

`_u_dot`\\
Time derivative of the associated nonlinear variable

`_du_dot_du`\\
The derivative of _u_dot with respect to _u

!---

## TimeKernel Base Classes

| Base | Override | Use |
| :- | :- | :- |
| TimeKernel\\ +ADTimeKernel+ | computeQpResidual | Use when the time term in the [!ac](PDE) is multiplied by both the test function and the gradient of the test function (`_test` and `_grad_test` must be applied) |
| TimeKernelValue\\ +ADTimeKernelValue+ | precomputeQpResidual | Use when the time term computed in the [!ac](PDE) is only multiplied by the test function (do not use `_test` in the override, it is applied automatically) |
| TimeKernelGrad\\ +ADTimeKernelGrad+ | precomputeQpResidual | Use when the time term computed in the [!ac](PDE) is only multiplied by the gradient of the test function (do not use `_grad_test` in the override, it is applied automatically) |

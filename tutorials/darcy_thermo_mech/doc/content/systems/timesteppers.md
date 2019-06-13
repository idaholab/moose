#  Time Stepper System

A system for suggesting time steps for transient executioners.

!---

!listing adapt_tstep_grow_dtfunc.i block=Executioner

Custom objects are created by inheriting from `TimeStepper` overriding `computeDT()`.

!---

## Built-in TimeSteppers

MOOSE includes many built-in TimeStepper objects:

- `ConstantDT`
- `SolutionTimeAdaptiveDT`
- `IterationAdaptiveDT`
- `FunctionDT`
- `PostprocessorDT`
- `DT2`
- `TimeSequenceStepper`

!---

## IterationAdaptiveDT

IterationAdaptiveDT grows or shrinks the time step based on the number of iterations taken to obtain
a converged solution in the last converged step.

!listing adapt_tstep_shrink_init_dt.i block=Executioner

!---

## DT2 Adaptive TimeStepper

1. Take one time step of size $\Delta t$ to get $\hat u_{n+1}$ from $u_n$
1. Take two time steps of size $\frac{\Delta t}{2}$ to get $u_{n+1}$ from $u_n$
1. Calculate local relative time discretization error estimate

   !equation
   \hat e_n \equiv \frac{\|u_{n+1} - \hat u_{n+1}\|_2}{\max (\|u_{n+1}\|_2, \|\hat u_{n+1}\|_2)}

1. Obtain global relative time discretization error estimate $e_n \equiv \frac{\hat e_n}{\Delta t}$
1. Adaptivity is based on target error tolerance $e_{TOL}$ and a maximum acceptable error tolerance $e_{MAX}$.

   - If $e_{n} < e_{MAX}$, continue with a new time step size

     !equation
     \Delta t_{n+1} \equiv \Delta t_n \cdot \left(\frac{e_{TOL}}{e_n} \right)^{1/p}

     where $p$ is the global convergence rate of the time stepping scheme.
   - If $e_{n} \ge e_{MAX}$, or if the solver fails, shrink $\Delta t$.

Parameters $e_{TOL}$ and $e_{MAX}$ can be specified in the input file as `e_tol` and `e_max` (in the `Executioner` block).

!---

## TimeSequenceStepper

Provide a vector of time points using parameter `time_sequence`, the object simply moves through
these time points.

The $t_{start}$ and $t_{end}$ parameters are automatically added to the sequence.

Only time points satisfying $t_{start} < t <t_{end}$ are considered.

If a solve fails at step $n$ an additional time point $t_{new} = \frac{1}{2}(t_{n+1}+t_n)$ is
inserted and the step is resolved.

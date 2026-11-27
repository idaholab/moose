# SystemSolveExecutor

!syntax description /Executors/SystemSolveExecutor

## Overview

`SystemSolveExecutor` solves the solver system given in
[!param](/Executors/SystemSolveExecutor/system). The system can be nonlinear or linear. A
[SteadyExecutor.md] or another executor runs it as an inner executor.

The settings of the executor apply to its system only:

- The PETSc options get the PETSc prefix of the system. With more than one solver system, the
  prefix is the system name followed by `_`. Give the options without the prefix.
- The solve type, the line search and the finite differencing type apply to a nonlinear system
  only. The executor reports an error if they are set for a linear system.
- The linear solver tolerances apply to the linear solves of the system.
- [!param](/Executors/SystemSolveExecutor/convergence) sets the [Convergence](Convergence/index.md)
  object of the system. Without it, a nonlinear system uses the default nonlinear Convergence
  object, and a linear system uses the linear solver tolerances.

[!param](/Executors/SystemSolveExecutor/relaxation_factor) relaxes a converged solution with the
solution of the previous multi-system fixed point iteration. A relaxation factor other than 1
requires a [FixedPointExecutor.md] with this executor in its inner executors.

After the solve, the executor prints whether the system converged. The executor fails if the
system does not converge. After the solve of a linear system, the executor computes the gradients
of its variables.

## Example input syntax

!listing test/tests/executors/system_solve/executors.i block=Executors

!syntax parameters /Executors/SystemSolveExecutor

!syntax inputs /Executors/SystemSolveExecutor

!syntax children /Executors/SystemSolveExecutor

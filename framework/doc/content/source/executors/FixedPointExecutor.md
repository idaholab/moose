# FixedPointExecutor

!syntax description /Executors/FixedPointExecutor

## Overview

`FixedPointExecutor` runs a multi-system fixed point iteration. Each iteration does these steps:

1. Print the iteration number and call `preIteration` on the
   [!param](/Executors/FixedPointExecutor/convergence) object.
1. Store the solution of each system that needs its previous iteration, for example for
   relaxation.
1. Run the executors in [!param](/Executors/FixedPointExecutor/inner_executors). The iteration
   stops at the first executor that does not converge.
1. Execute and output the `MULTISYSTEM_FIXED_POINT_ITERATION_END` objects.
1. Check the [Convergence](Convergence/index.md) object. The executor fails if the iteration
   diverges.

Without a [!param](/Executors/FixedPointExecutor/convergence) object, the executor does one sweep
and prints no iteration lines.

The inner executors can be of any type. A [SystemSolveExecutor.md] relaxes the solution of its
system with its own [!param](/Executors/SystemSolveExecutor/relaxation_factor).

[!param](/Executors/FixedPointExecutor/sweep_type) sets the order of one sweep over the N inner
executors:

- `multiplicative` runs the executors in order, 1 to N.
- `symmetric_multiplicative` then runs them in reverse order, N-1 to 1. The executor N does not
  run again, because its inputs have not changed.

## Example input syntax

!listing test/tests/executors/fixed_point/mixed_systems.i block=Executors

!syntax parameters /Executors/FixedPointExecutor

!syntax inputs /Executors/FixedPointExecutor

!syntax children /Executors/FixedPointExecutor

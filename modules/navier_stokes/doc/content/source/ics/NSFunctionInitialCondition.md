# NSFunctionInitialCondition

`NSFunctionInitialCondition` can be used to initialize variables by evaluating fluid properties,
using [functions](syntax/Functions/index.md) for the pressure, temperature and velocity.

## Example input file syntax

In this example, we initialize the density at the desired initial temperature and pressure conditions.

!listing modules/navier_stokes/test/tests/ics/test_function.i block=ICs/rho

!syntax parameters /ICs/NSFunctionInitialCondition

!syntax inputs /ICs/NSFunctionInitialCondition

!syntax children /ICs/NSFunctionInitialCondition

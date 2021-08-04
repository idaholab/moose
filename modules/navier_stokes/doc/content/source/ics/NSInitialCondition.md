# NSInitialCondition

`NSInitialCondition` can be used to initialize variables by evaluating fluid properties.

## Example input file syntax

In this example, we initialize the density at the desired initial temperature and pressure conditions.

!listing modules/navier_stokes/test/tests/ics/test.i block=ICs/rho

!syntax parameters /ICs/NSInitialCondition

!syntax inputs /ICs/NSInitialCondition

!syntax children /ICs/NSInitialCondition

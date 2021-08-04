# PNSInitialCondition

`PNSInitialCondition` can be used to initialize variables by evaluating fluid properties
for porous flow calculations. It can accept both interstitial and superficial velocities for
initializing superficial or interstitial velocity variables and energy.

## Example input file syntax

In this example, we initialize the first component of the superficial momentum with a specified superficial velocity. The superficial velocity is specified as a vector. We could have alternatively specified
an interstitial velocity and the porosity.

!listing modules/navier_stokes/test/tests/ics/pns_test.i block=ICs/superficial_rhou

!syntax parameters /ICs/PNSInitialCondition

!syntax inputs /ICs/PNSInitialCondition

!syntax children /ICs/PNSInitialCondition

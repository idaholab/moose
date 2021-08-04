# PNSInitialCondition

`PNSInitialCondition` can be used to initialize variables by evaluating fluid properties
for porous flow calculations. It can accept both interstitial and superficial velocities for
initializing superficial or interstitial velocity variables and energy.

## Example input file syntax

In this example, we initialize the first component of the superficial momentum with a specified superficial velocity.

!listing modules/navier_stokes/test/tests/ics/cns_test.i block=ICs/superficial_rhou

!syntax description /ICs/PNSInitialCondition

!syntax parameters /ICs/PNSInitialCondition

!syntax inputs /ICs/PNSInitialCondition

!syntax children /ICs/PNSInitialCondition

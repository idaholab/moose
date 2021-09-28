# BoundingValueNodalDamper

!syntax description /Dampers/BoundingValueNodalDamper

The value of the damper is modified at every quadrature point based on
how far out of the defined bounds the variable is there, and in which direction
the variable is 'incremented' between successive iterations, away or closer to the bound.
The damping increases when the variable is moving further away from the prescribed bounds.

More information about dampers may be found on the
[Dampers syntax documentation](syntax/Dampers/index.md).

## Example input syntax

In this example, the variable `u` is being damped with minimum and maximum values of -1.5 and
1.5 respectively, with a minimum damping factor of 0.001.

!listing test/tests/dampers/min_damping/min_nodal_damping.i block=Dampers

!syntax parameters /Dampers/BoundingValueNodalDamper

!syntax inputs /Dampers/BoundingValueNodalDamper

!syntax children /Dampers/BoundingValueNodalDamper
